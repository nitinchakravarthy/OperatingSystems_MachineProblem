/*
 File: ContFramePool.C
 
 Author:
 Date  : 
 
 */

/*--------------------------------------------------------------------------*/
/* 
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates 
 *single* frames at a time. Because it does allocate one frame at a time, 
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.
 
 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free 
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.
 
 This can be done in many ways, ranging from extensions to bitmaps to 
 free-lists of frames etc.
 
 IMPLEMENTATION:
 
 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame, 
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool. 
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.
 
 NOTE: If we use this scheme to allocate only single frames, then all 
 frames are marked as either FREE or HEAD-OF-SEQUENCE.
 
 NOTE: In SimpleFramePool we needed only one bit to store the state of 
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you 
 
 get this to work, 
 revisit the implementation and change it to using two bits. You will get 
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.
 
 DETAILED IMPLEMENTATION:
 
 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:
 
 Constructor: Initialize all frames to FREE, except for any frames that you 
 need for the management of the frame pool, if any.
 
 get_frames(_n_frames): Traverse the "bitmap" of states and look for a 
 sequence of at least _n_frames entries that are FREE. If you find one, 
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or 
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.
 
 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.
 
 needed_info_frames(_n_frames): This depends on how many bits you need 
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.
 
 A WORD ABOUT RELEASE_FRAMES():
 
 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e., 
 not associated with a particular frame pool.
 
 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete
 
 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/
#define MB * (0x1 << 20)
#define KB * (0x1 << 10)
/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */
ContFramePool* ContFramePool::cont_frame_pool_head;
ContFramePool* ContFramePool::cont_frame_pool_list;
/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/



/*
The following are the bit conventions used in this code
Free 			   -00
Allocated and Head -10
Allocated not Head -11
Not Accessible     -01
*/
ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no,
                             unsigned long _n_info_frames)
{
    // TODO: IMPLEMENTATION NEEEDED!
    //assert(false);

    assert(_n_frames <= FRAME_SIZE*8);

    base_frame_no = _base_frame_no;
    nframes = _n_frames;
    nFreeFrames = _n_frames;
    info_frame_no = _info_frame_no;
    n_info_frames = _n_info_frames;

    // If _info_frame_no is zero then we keep management info in the first
    //frame, else we use the provided frame to keep management info
    if(info_frame_no == 0) {
		bitmap = (unsigned char *) (base_frame_no * FRAME_SIZE);
    } else {
		bitmap = (unsigned char *) (info_frame_no * FRAME_SIZE);
    }

    // Number of frames must be "fill" the bitmap!
    assert ((nframes % 8 ) == 0);

    // Mark all the bits in the bitmap
    for(int i = 0; i*4 < _n_frames; i++){
    	bitmap[i] = 0x00;
    };

 	if(_info_frame_no == 0) {
 		if(_n_info_frames > 0){
 			mark_inaccessible(_info_frame_no, _n_info_frames);
 		} else{
 			bitmap[0] = 0x40;	// marking inaccessible
 			nFreeFrames--;
 		}
        
        
    }
	    
	    if (ContFramePool::cont_frame_pool_head == NULL){
	    	ContFramePool::cont_frame_pool_head = this;
	    	ContFramePool::cont_frame_pool_list = this;
	    }else{
	    	ContFramePool::cont_frame_pool_list -> frame_pool_next = this;
	    	ContFramePool::cont_frame_pool_list = this;
	    }
	    
	    Console::puts("Frame Pool initialized\n"); 
  
}

unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{

    // TODO: IMPLEMENTATION NEEEDED!
    //assert(false);
    
    // Make sure the number of free frames is greater that than the needed number of frames
    assert(nFreeFrames > _n_frames);
	

	unsigned int head_frame_no = base_frame_no;
	unsigned int byte_index=0;
	unsigned int bit_index=0;
	
	// we keep updating them when we find a first empty frame
	unsigned int tempHead_byteIndex =0;
	unsigned int tempHead_bitIndex = 0;
	unsigned int tempHeadFound = 0;		// tempHead not found yet. That would be the first found free frame
	unsigned int frames_to_be_found = _n_frames;

	for (int i = 0 ; i < nframes/4 ; i++){
		unsigned char currentByte = bitmap[i];
		unsigned char filter_mask = 0xC0; //11000000

		for (int j =0; j < 4 ; j++) {
			if(tempHeadFound == 0){
				// first find the tempHead
				if((bitmap[i] & filter_mask) == 0){ 		// this gives the 2 bits needed and the rest as zeros.
			    	tempHeadFound = 1;
					byte_index =i;
					bit_index = j;
					head_frame_no += (4*i + j);	
					frames_to_be_found--;
				}
				filter_mask = filter_mask >> 2;

			} else {
				// now see if you can get _n_frames-1 frames after the temp head
				if (frames_to_be_found > 0){
					if((bitmap[i] & filter_mask) == 0){
					// found a free frame
						frames_to_be_found--;
						filter_mask = filter_mask >>2;

					} else {
						// non free frame - now reset back to original values;
						byte_index =0;
						bit_index = 0;
						head_frame_no = base_frame_no;	
						tempHeadFound = 0;
						frames_to_be_found = _n_frames;
						filter_mask = filter_mask>>2;
					}
				} else {
					break;
				}
			}	
			
		}
		if (frames_to_be_found == 0){
			// break the outer loop.
			break;
		}
	}
	
	// Now see if we got the contiguous block of memory;
	if (frames_to_be_found != 0){
		// frames not found
		return 0;
	}

	// Now you have a contiguous piece of memory that starts from bit_index of byte_index till _n_frames from there.

	unsigned int to_be_marked = _n_frames;
	// Marking head
	unsigned char head_byte = bitmap[byte_index];
	unsigned char head_mask = 0x80;			// 10000000
	head_mask = head_mask >>(2*bit_index);	// 10 moves to the needed place; (eg: 00100000)
	bitmap[byte_index] = bitmap[byte_index] | head_mask;	// only the head becomes 10, other will remain the same

	to_be_marked--;
	bit_index++;
	
	// after head in the same byte
	unsigned char not_head_mask = 0xC0;
	not_head_mask = not_head_mask >> 2*bit_index;
	while(to_be_marked > 0 && bit_index < 4) {
        bitmap[byte_index] = bitmap[byte_index] | not_head_mask;
        not_head_mask = not_head_mask >> 2;
        to_be_marked--;
        bit_index++;
    }
	
	// from next bit till to_be_marked == 0;
	for (int i = byte_index+1; i < nframes/4; i++){
		not_head_mask = 0xC0;
		for (int j = 0; j < 4; j++) {
			if(to_be_marked == 0){
				break;
			}
			bitmap[i] = bitmap[i] | not_head_mask;
			to_be_marked--;
			not_head_mask = not_head_mask >> 2;

		}
		if(to_be_marked == 0){
			break;
		}
	}

	
	nFreeFrames -= _n_frames;
	//Console::puts("Head Frame Number is:" );Console::puti(head_frame_no);Console::puts("\n");
	return head_frame_no;
	
}

// we mark inacessible just after the constructor is made. before assigning memory to any process.
// So all the entries are Zero and we want one frame entry to be 01 --> eg:(00000000 --> 00010000)
// mask is 01000000 & 00000000(abcdefgh) ==>   
void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{
    // TODO: IMPLEMENTATION NEEEDED!
    //assert(false);
     for(int i = _base_frame_no; i < _base_frame_no + _n_frames; i++){
        mark_inaccessible(i);
    }
    
}

void ContFramePool::mark_inaccessible(unsigned long _frame_no)
{

	// Let's first do a range check.
    assert ((_frame_no >= base_frame_no) && (_frame_no < base_frame_no + nframes));
    
    unsigned int frame_byte_no = (_frame_no - base_frame_no)/4;
    unsigned int frame_bit_no = (_frame_no - base_frame_no) %4;
    
    unsigned char filter_mask = 0xC0;
    filter_mask = filter_mask >> 2* frame_bit_no;
    unsigned char inacess_mask = 0x80;
    inacess_mask = inacess_mask >> 2*frame_bit_no;
    
    assert((bitmap[frame_byte_no] & filter_mask) == 0 );
    bitmap[frame_byte_no] = ( bitmap[frame_byte_no] | filter_mask) ^ inacess_mask; // filter the needed to bit and xor with inacessmask
    nFreeFrames--;
    
}


void ContFramePool::release_frames(unsigned long _first_frame_no)
{
    // TODO: IMPLEMENTATION NEEEDED!
    //assert(false);
    // first find the corresponding ContFramePool
    //ContFramePool* current_pool = ContFramePool::cont_frame_pool_head;
    ContFramePool* current_pool = ContFramePool::cont_frame_pool_head;
    while ( (current_pool->base_frame_no > _first_frame_no || current_pool->base_frame_no + current_pool->nframes <= _first_frame_no) ) {
        if (current_pool->frame_pool_next == NULL) {
            Console::puts("Frame not found in any pool. \n");
            return;
        } else {
            current_pool = current_pool->frame_pool_next;
        }
    }
    
    //current_pool::release_frames(_first_frame_no, current_pool);
    unsigned char * bitmap = current_pool->bitmap;
	unsigned int frames_diff = _first_frame_no - current_pool->base_frame_no;
	unsigned int byte_index = (frames_diff)/4;
	unsigned int bit_index = (frames_diff)%4;
	
	
	// check if the given frame is head.
	unsigned char head_mask = 0x40;
	unsigned char bit_mask = 0xC0;		// found these values by experimentation
	head_mask = head_mask>>bit_index*2;
    bit_mask = bit_mask>>bit_index*2;
	if (((bitmap[byte_index] ^ head_mask) & bit_mask) == bit_mask){
		// head in the byte
		
		// make head free
		bitmap[byte_index] = bitmap[byte_index] & (~bit_mask);
		current_pool->nFreeFrames++;
		bit_index++;
		bit_mask = bit_mask >> 2;
		
		
		// now for the other frames in the same byte.
		while (bit_index < 4){
			if (bitmap[byte_index] & bit_mask == bit_mask){
				bitmap[byte_index] == bitmap[byte_index] & (~bit_mask);
				bit_mask = bit_mask >>2;
				bit_index++;
				current_pool->nFreeFrames++;
			}else{
				return;
			}
		}
		
		// now the frames in the next bytes till the end.
		for(int i = byte_index+1; i < (current_pool->base_frame_no + current_pool->nframes)/4; i++ ) {
			bit_mask = 0xC0;
			for(int j=0; j < 4; j++) {
				if (bitmap[i] && bit_mask == bit_mask){
					bitmap[byte_index] == bitmap[i] & (~bit_mask);
					bit_mask = bit_mask >>2;
					bit_index++;
					current_pool->nFreeFrames++;
				}else{
					return;	
				}
			}// bit loop
		}// byte loop
		
		
	} else {
		Console::puts("Not a head of sequence\n");
	}
		
}

unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
    //assert(false);
    return (_n_frames*2)/(8*4 KB) + ((_n_frames*2) % (8*4 KB) > 0 ? 1 : 0);
}

