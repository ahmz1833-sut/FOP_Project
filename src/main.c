#include "main.h"

int main(int argc, char const *argv[])
{
    prepareANSI();

    printf(_SGR_DIM "The Number of arguments is : \"%d\" \n", argc);
    for(int i = 0; i < argc; i++) printf("The Argument[%d] is : \"%s\" \n", i, argv[i]);
    printf(_SGR_RESET);

    if(argc >= 2 && !strcmp(argv[1], "--uninstall")) // Uninstall Command
        promptUninstallation();
    
    else if(argc >= 2 && !strcmp(argv[1], "--install")) // Install Command (for upgrade)
        promptInstallation(argv[0]);
    
    else if((strcmp(argv[0], PROGRAM_NAME) != 0) && !IS_INSTALLED()) // NeoGIT is not installed!!
        promptInstallation(argv[0]);
    
    
    int i = 0;
    printf("salam %d\n", i);
    return 0;
}
