//inline unsigned long long rdtsc_fenced(void);
unsigned long long rdtsc_fenced(void);
unsigned long long rdtsc_fenced(void) {
    unsigned long long low, high;
    
    asm volatile("lfence \n"
                 "rdtsc \n"
                 :  "=a" (low), "=d"(high)
                 :
                 :"memory" );
    return (low | (high << 32));
}
