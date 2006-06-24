#ifndef _LIBSIM_H
#define _LIBSIM_H

/* Memory-mapped registers */
#define SIM_OUTPUT_REG 0xE000
#define SIM_EXIT_REG 0xE001
#define SIM_INPUT_REG 0xE002

/* Function prototypes */
void sys_console_write (char c);
char sys_console_read (void);

#endif /* _LIBSIM_H */
