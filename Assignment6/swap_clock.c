#include <assert.h>
#include <defs.h>
#include <riscv.h>
#include <stdio.h>
#include <string.h>
#include <swap.h>
#include <swap_clock.h>
#include <list.h>
#include <pmm.h>


list_entry_t pra_list_head, *curr_ptr;

static int
_clock_init_mm(struct mm_struct *mm)
{   
    // init the list like lab 13 slide
    list_init(&pra_list_head);

    // swap manager's private data
    mm->sm_priv = &pra_list_head;

    // set ptr to head
    curr_ptr = (list_entry_t*) mm->sm_priv;

    return 0;
}

static int
_clock_map_swappable(struct mm_struct *mm, uintptr_t addr, struct Page *page, int swap_in)
{   
    // find the head list and target list entry
    list_entry_t *entry =& (page->pra_page_link);
    assert(entry != NULL);
    assert(curr_ptr != NULL);

    // record the page access situlation
    // link the new page to previous page of current pointer of the pra_list_head qeueue.
    list_add_before(curr_ptr, entry);

    return 0;
}


static int
_clock_swap_out_victim(struct mm_struct *mm, struct Page ** ptr_page, int in_tick)
{   
    // by lab 13 slide, check the head is leagal
    list_entry_t *head = (list_entry_t*) mm->sm_priv;
    assert(head != NULL);
    assert(curr_ptr != NULL);
    assert(in_tick == 0);

    // forever find list entry
    while(1) {
        // if head, to next
        if(curr_ptr == head) {
            curr_ptr = list_next(curr_ptr);
        }
        // get ptr_page
        *ptr_page = le2page(curr_ptr, pra_page_link);
        // get pte
        pte_t* ptep = get_pte(mm->pgdir, (*ptr_page)->pra_vaddr, 0);
        
        // get access state
        int accessed = *ptep & PTE_A;
        if (accessed) {
            // set accessd to 0
            *ptep &= ~PTE_A;
        } else {
            // delete the entry
            curr_ptr = list_next(curr_ptr);
            list_del(list_prev(curr_ptr));
            break;
        }
        // search to next
        curr_ptr = list_next(curr_ptr);
    }

    return 0;
}


static int
_clock_check_swap(void) {

    cprintf("---------Clock check begin----------\n");
    cprintf("write Virt Page c in clock_check_swap\n");
    *(unsigned char *)0x3000 = 0x0c;
    assert(pgfault_num==4);
    cprintf("write Virt Page a in clock_check_swap\n");
    *(unsigned char *)0x1000 = 0x0a;
    assert(pgfault_num==4);
    cprintf("write Virt Page d in clock_check_swap\n");
    *(unsigned char *)0x4000 = 0x0d;
    assert(pgfault_num==4);
    cprintf("write Virt Page b in clock_check_swap\n");
    *(unsigned char *)0x2000 = 0x0b;  
    assert(pgfault_num==4);
    cprintf("write Virt Page e in clock_check_swap\n");
    *(unsigned char *)0x5000 = 0x0e;
    assert(pgfault_num==5);
    cprintf("write Virt Page b in clock_check_swap\n");
    *(unsigned char *)0x2000 = 0x0b;
    assert(pgfault_num==5);
    cprintf("write Virt Page a in clock_check_swap\n");
    *(unsigned char *)0x1000 = 0x0a;
    assert(pgfault_num==6);
    cprintf("write Virt Page b in clock_check_swap\n");
    *(unsigned char *)0x2000 = 0x0b;
    assert(pgfault_num==6);
    cprintf("write Virt Page c in clock_check_swap\n");
    *(unsigned char *)0x3000 = 0x0c;
    assert(pgfault_num==7);
    cprintf("write Virt Page d in clock_check_swap\n");
    *(unsigned char *)0x4000 = 0x0d;
    assert(pgfault_num==8);
    cprintf("write Virt Page e in clock_check_swap\n");
    *(unsigned char *)0x5000 = 0x0e;
    assert(pgfault_num==9);
    cprintf("write Virt Page a in clock_check_swap\n");
    assert(*(unsigned char *)0x1000 == 0x0a);
    *(unsigned char *)0x1000 = 0x0a;
    assert(pgfault_num==9);
    cprintf("Clock check succeed!\n");

    return 0;
}


static int
_clock_init(void)
{
    return 0;
}

static int
_clock_set_unswappable(struct mm_struct *mm, uintptr_t addr)
{
    return 0;
}

static int
_clock_tick_event(struct mm_struct *mm)
{ return 0; }


struct swap_manager swap_manager_clock =
{
     .name            = "clock swap manager",
     .init            = &_clock_init,
     .init_mm         = &_clock_init_mm,
     .tick_event      = &_clock_tick_event,
     .map_swappable   = &_clock_map_swappable,
     .set_unswappable = &_clock_set_unswappable,
     .swap_out_victim = &_clock_swap_out_victim,
     .check_swap      = &_clock_check_swap,
};
