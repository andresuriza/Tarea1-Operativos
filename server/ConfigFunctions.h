#ifndef CONFIG_FUNCTIONS_H
#define CONFIG_FUNCTIONS_H

// Si solo se usa internamente, mejor que esté static en el .c y no aquí
// extern const char *filename; ← ¡NO pongas esto si lo defines en múltiples .c!

int GetPort(void);
char* Get_Dircolores(void);
char* Get_Dirhisto(void);
void WriteLog(const char *msg, ...);

#endif
