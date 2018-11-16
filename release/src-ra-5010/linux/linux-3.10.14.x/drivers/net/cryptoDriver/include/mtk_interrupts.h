
#ifndef EIP93_INTERRUPTS_H
#define EIP93_INTERRUPTS_H

// EIP93 interrupt signals
// assigned values represent interrupt source bit numbers
enum
{
    IRQ_CDR_THRESH_IRQ = 0,
    IRQ_RDR_THRESH_IRQ = 1,
    IRQ_OPERATION_DONE_IRQ = 9,
    IRQ_INBUF_THRESH_IRQ = 10,
    IRQ_OUTBUF_THRESH_IRQ = 11,
    IRQ_PRNG_IRQ=12,
    IQ_PE_ERR_IRQ = 13
};



#endif 

