#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;



void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
   //assert(false);

    PageTable::kernel_mem_pool = _kernel_mem_pool;
    PageTable::process_mem_pool = _process_mem_pool;
    PageTable::shared_size = _shared_size;

    Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
   //assert(false);

    page_directory = (unsigned long *)(kernel_mem_pool->get_frames(1)*PAGE_SIZE);

    unsigned long mem_start = 0;
    unsigned long * pg_tab = (unsigned long *)(kernel_mem_pool->get_frames(1)*PAGE_SIZE);

    unsigned long shared_frames = ( PageTable::shared_size / PAGE_SIZE);
    //Console::puts("shared frames : "); Console::puti(shared_frames);

    for(int i = 0; i < shared_frames; i++) {
        pg_tab[i] = mem_start |3 ;
        mem_start += PAGE_SIZE;
    }

    //set the first page entry
    page_directory[0] = (unsigned long)pg_tab | 3;

    unsigned long mask = 0;
    //mark the non shared memory page entries
    for(int i = 1; i< shared_frames; i++) {
        page_directory[i] = mask | 2;
    }

    Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
   //assert(false);

    current_page_table = this;
    write_cr3((unsigned long)page_directory);
    Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
   //assert(false);

    paging_enabled = 1;
    write_cr0(read_cr0() | 0x80000000);
    Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{

	unsigned long PG_DIR_MASK = 0xFFFFF000;	// to take only first 20 bits of 32 bits.
	
    unsigned long error_code = _r->err_code;
    unsigned long * pg_dir = (unsigned long *) read_cr3();		//page directory
    
    unsigned long fault_addr = read_cr2();		// address where fault occured. (10-10-12)
    unsigned long pg_dir_addr   = fault_addr >> 22;
    unsigned long pg_tab_addr   = fault_addr >> 12 & 0x3FF;

    unsigned long * pg_tab = NULL;

    if ((error_code & 1) == 0 ) {
    	
    	if((pg_dir[pg_dir_addr] & 1) == 0){
    		pg_dir[pg_dir_addr] = (unsigned long) ((kernel_mem_pool->get_frames(1)*PAGE_SIZE) | 3);
            pg_tab = (unsigned long *)(pg_dir[pg_dir_addr] & PG_DIR_MASK );
            for (int i = 0; i<PAGE_SIZE; i++) {
                pg_tab[i] = 0 ; // just making everying to zero initially.
            }
            pg_tab[pg_tab_addr] = (PageTable::process_mem_pool->get_frames(1)*PAGE_SIZE) | 3 ;
    	} else{
    		pg_tab = (unsigned long *)(pg_dir[pg_dir_addr] & PG_DIR_MASK );
        	pg_tab[pg_tab_addr] = (PageTable::process_mem_pool->get_frames(1)*PAGE_SIZE) | 3 ;
    	}
    }

    Console::puts("handled page fault\n");
}

