#pragma once

/*Sleep(6000);
while (1) {
    uint32_t fflag;
    ReadProcessMemory(hProcess, LPVOID(local.pawnAddr + cs2_dumper::schemas::client_dll::C_BaseEntity::m_fFlags), &fflag, sizeof(fflag), nullptr);
    uint32_t jump;
    uint32_t setjump = 65537;
    uint32_t setjump1 = 16777472;
    ReadProcessMemory(hProcess, LPVOID((uint64_t)ClientModule + cs2_dumper::buttons::jump), &jump, sizeof(jump), nullptr);
    std::cout << fflag << std::endl;
    std::cout << jump << std::endl;
    if (fflag == 65665) {
        Sleep(13);
        WriteProcessMemory(hProcess, LPVOID((uint64_t)ClientModule + cs2_dumper::buttons::jump), &setjump, sizeof(setjump), nullptr);
    }
    else {
        WriteProcessMemory(hProcess, LPVOID((uint64_t)ClientModule + cs2_dumper::buttons::jump), &setjump1, sizeof(setjump1), nullptr);
    }


    system("cls");
}*/