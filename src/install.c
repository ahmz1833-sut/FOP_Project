#include "install.h"

int promptInstallation(const char* srcAddress)
{
    if(!IS_INSTALLED()) // First Installation
    {
        printf(_SGR_YELLOWF "The NeoGIT is not found in PATH for launching commands!\n");
        printf(_SGR_DEF_FG _SGR_BOLD "Do you want to install NeoGIT in a system folder in PATH? (Y/N): ");
    }
    else // Upgrade
    {
        printf(_SGR_YELLOWF "The NeoGIT is already installed in your system!\n");
        printf(_SGR_DEF_FG _SGR_BOLD "Do you want to upgrade NeoGIT ? (Y/N): ");
    }    

    char result;
    scanf("%1[YNyn]", &result);
    printf(_SGR_RESET);

    if(toupper(result) == 'Y')
    {
        printf("\nInstalling NeoGIT in " PROGRAM_PATH " ...\n");
        int res = INSTALL_NEOGIT(srcAddress);
        if(res != 0) perror(_SGR_REDF _SGR_BOLD "Failed to install\n\n" _SGR_RESET);
        else printf(_SGR_GREENF _SGR_BOLD "Successfully installed\n\n" _SGR_RESET);
        return res;
    }
    else 
        printf(_SGR_BLUEF "Installation Cancelled\n\n" _SGR_RESET);
    return -1;
}

int promptUninstallation()
{
    if(IS_INSTALLED())
    {
        printf(_SGR_BOLD _SGR_YELLOWF "NeoGIT found in PATH! Are you sure to remove it? (Y/N): " _SGR_RESET);
        char result;
        scanf("%1[YNyn]", &result);
        if(toupper(result) == 'Y')
        {
            printf("\nRemoving NeoGIT ...\n");
            int res = REMOVE_NEOGIT();
            if(res != 0) perror(_SGR_REDF _SGR_BOLD "Error while removing NeoGIT!\n\n" _SGR_RESET);
            else printf(_SGR_GREENF _SGR_BOLD "Removed Successfully!\n\n" _SGR_RESET);
            exit(res);
        }
        else
            printf(_SGR_BLUEF "Operation Cancelled\n\n" _SGR_RESET);
    }
    else printf(_SGR_YELLOWF "NeoGIT was not installed!\n\n" _SGR_RESET);

    exit(2);
}

#ifdef _WIN32 // For Windows

int __install_neogit_win32(const char* source)
{
    // Upgrade
    if(IS_INSTALLED()) REMOVE_NEOGIT();

    char __directoryPath[200];
    char* ptr = strrchr(source, '\\');
    if (ptr != NULL)
    {
        uint len = (ptr - source);
        sprintf(__directoryPath, "%s", source);
        __directoryPath[len] = '\\';
        __directoryPath[len + 1] = '\0';
    }
    else strcpy(__directoryPath, ".\\");

    char _prompt[300];
    sprintf(_prompt, "cd \"%s\"", __directoryPath);
    if(system(_prompt) != 0) return -1;
    *(strstr(_prompt, ":\\")+1) = '\0';
    if(system(_prompt+3) != 0) return -1;
    if(system("Echo Set UAC = CreateObject^(\"Shell.Application\"^) > _tmp.vbs") != 0) return -1;
    sprintf(_prompt, "Echo UAC.ShellExecute \"cmd\", \"/c copy \" ^& Chr(34) ^& \"%s" PROGRAM_NAME ".exe\" ^& Chr(34) ^& \" " PROGRAM_PATH " \", \"\", \"runas\", 1 >> _tmp.vbs", __directoryPath);
    if(system(_prompt) != 0) return -1;
    if(system("cscript _tmp.vbs >nul 2>&1") != 0) return -1;
    remove("_tmp.vbs");
    sleep(1);
    return IS_INSTALLED() ? 0 : -1;
}

int __remove_neogit_win32()
{
    if(IS_INSTALLED())
    {
        if(system("Echo Set UAC = CreateObject^(\"Shell.Application\"^) > _tmp.vbs") != 0) return -1;
        if(system("Echo UAC.ShellExecute \"cmd\", \"/c del " PROGRAM_PATH "\", \"\", \"runas\", 1 >> _tmp.vbs")) return -1;
        if(system("cscript _tmp.vbs >nul 2>&1") != 0) return -1;
        system("del _tmp.vbs >nul 2>&1");
        sleep(1);
        return IS_INSTALLED() ? -1 : 0;
    }
    return 0;
}

#else // For Linux

int __install_neogit_linux(const char* source)
{
    // Upgrade
    if(IS_INSTALLED()) REMOVE_NEOGIT();

    char __directoryPath[200];
    char* ptr = strrchr(source, '/');
    if (ptr != NULL)
    {
        uint len = (ptr - source);
        sprintf(__directoryPath, "%s", source);
        __directoryPath[len] = '/';
        __directoryPath[len + 1] = '\0';
    }
    else strcpy(__directoryPath, "./");
    
    char _prompt[300];
    sprintf(_prompt, "sudo ln -s \"$(pwd)/%s/" PROGRAM_NAME "\" " PROGRAM_PATH, __directoryPath);
    if(system(_prompt)) return -1;

    return IS_INSTALLED() ? 0 : -1;
}

int __remove_neogit_linux()
{
    if(!IS_INSTALLED()) return 0;
    if(system("sudo rm " PROGRAM_PATH " >/dev/null 2>&1")) return -1;
    return IS_INSTALLED() ? -1 : 0;
}

#endif