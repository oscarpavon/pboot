#include <efi/efi.h>
#include <efi/efilib.h>
EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
    Print(L"Hi,,,");
    
    while(1){

    }
    return EFI_SUCCESS;
}

