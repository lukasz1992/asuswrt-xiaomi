/*
**
*/
int LoadIODLL_C();
void PortOut_C(short int addr,char data);
char PortIn_C (short int addr);
int PortAddress_C (unsigned char LPT_Number);
int IsDriverInstalled_C ();
void UnloadIODLL_C ();