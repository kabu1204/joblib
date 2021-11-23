//
// Created by yuchengye on 2021/11/23.
//

#ifndef JOBLIB_USER_STACK_HPP
#define JOBLIB_USER_STACK_HPP

#ifdef __x86_64__
struct registers
{
    DWORD rdi;
    DWORD rbx;
    DWORD rbp;
    DWORD r12;
    DWORD r13;
    DWORD r14;
    DWORD r15;
    registers(){
        rdi=0;
        rbx=0;rbp=0;
        r12=0;r13=0;r14=0;r15=0;
    }
};
#endif

void stack_cleaner(){
    // do something
}

struct user_stack{
    DWORD *sp;
    DWORD *stack_space;
    registers regs;
public:
    typedef void (*FP_CALL_BACK)();
    user_stack(size_t stack_size=DEFAULT_CORO_STACK_SIZE, FP_CALL_BACK CALL_BACK_FUNC=nullptr){
        if(stack_size==0){
            sp=0;
            stack_space=0;
            return;
        }
        stack_space=new DWORD[DEFAULT_CORO_STACK_SIZE/sizeof(DWORD)+1]; // on macOS, plus 1 to avoid stack_not_16_byte_aligned_error
        sp=&stack_space[DEFAULT_CORO_STACK_SIZE/sizeof(DWORD)]; // on macOS, minus 2 to avoid stack_not_16_byte_aligned_error
        if(CALL_BACK_FUNC){
            *(--sp)=(DWORD)CALL_BACK_FUNC; // coroutine cleanup
        }
    }

    template<class T>
    void push(T content){
        *(--sp)=(DWORD)content;
    }

    template<class T>
    void pop(T& ret){
        ret=(T)(*sp);
        sp++;
    }

};

extern "C" void switch_user_context(user_stack* src_stack, user_stack* dst_stack);

#ifdef __x86_64__
__asm__(
".globl _switch_user_context"
"_switch_user_context:\n\t"
"movq %rdi,16(%rdi) \n\t"
"movq %rbx,24(%rdi) \n\t"
"movq %rbp,32(%rdi) \n\t"
"movq %r12,40(%rdi) \n\t"
"movq %r13,48(%rdi) \n\t"
"movq %r14,56(%rdi) \n\t"
"movq %r15,64(%rdi) \n\t"

"movq %rsp,(%rdi) \n\t"
"movq (%rsi),%rsp \n\t"

"movq 64(%rsi),%r15 \n\t"
"movq 56(%rsi),%r14 \n\t"
"movq 48(%rsi),%r13 \n\t"
"movq 40(%rsi),%r12 \n\t"
"movq 32(%rsi),%rbp \n\t"
"movq 24(%rsi),%rbx \n\t"
"movq 16(%rsi),%rdi \n\t"
//"popq %rdi \n\t"
//"addq $8,%rsp \n\t"
//"jmpq *-8(%rsp) \n\t"
"retq \n\t"
);
#endif

#endif //JOBLIB_USER_STACK_HPP
