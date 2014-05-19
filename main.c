#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include "vb6.h"

#define REPORT_PATH "C:\\logz.txt"
#define CUCKOO_PIPE "\\\\.\\PIPE\\cuckoo"

// address of msvbvm60.dll
static void *g_vb6;

void notify_cuckoo(const char *path)
{
    char buf[256]; uint32_t len;
    sprintf(buf, "FILE_NEW:%s", path);
    CallNamedPipe(CUCKOO_PIPE, buf, strlen(buf), buf, sizeof(buf),
        (DWORD *) &len, NMPWAIT_WAIT_FOREVER);
}

static int _generic_pre(uint32_t eax, uint32_t *esp,
    uint32_t *ebp, uint32_t *esi)
{
    (void) esp; (void) ebp;

    static vb6_insns_t *vb6_tables[] = {
        vb6_table_fb, vb6_table_fc, vb6_table_fd,
        vb6_table_fe, vb6_table_ff,
    };

    switch (eax) {
    case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
        report("%x %s", esi, vb6_tables[eax - 0xfb][*esi & 0xff].mnemonic);
        break;

    default:
        report("%x %s", esi, vb6_table_00[eax].mnemonic);
        break;
    }
    return 0;
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
    (void) hModule; (void) lpReserved;

    if(dwReason == DLL_PROCESS_ATTACH) {

        report_init(REPORT_PATH);
        notify_cuckoo(REPORT_PATH);

        g_vb6 = LoadLibrary("msvbvm60.dll");
        report("[+] msvbvm60.dll: 0x%x", g_vb6);

        if(vb6_hook_init(g_vb6) == 0) {
            vb6_hook_generic_table00(&_generic_pre);
        }
    }
    return TRUE;
}
