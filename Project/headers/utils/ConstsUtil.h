#ifndef CS2ASSIST_CONSTS_UTIL_H
#define CS2ASSIST_CONSTS_UTIL_H
#include <Windows.h>
#include <cmath>

namespace CS2Assist {
    namespace Consts {
        namespace Math {
            constexpr double PI = 3.14159265358979323846;
        }

        namespace Factor {
            constexpr float camera2head_Factor = 6.2f;
            constexpr float Fov_Angle = 95.8807f;
            constexpr float PerAngleInFov_scalingFactor = Fov_Angle * 1920 * 4 / 16200;
        }

        namespace SignCode {
            constexpr const char* strMapStateInfo = "E0 D4 ?? FD F8 7F 00 00 F8 D4 ?? FD F8 7F 00 00 10 D5 ?? FD F8 7F 00 00 00 00 00 00 00 00 00 00 ?? 00 00 00 00 01 00 C0";
            static constexpr size_t strMapStateInfo_size = 40;
            constexpr const char* BlueXray =       "32 C0 48 8B B4 24 ?? 00 00 00 48 8B 9C 24 ?? 00 00 00 48 8B AC 24 ?? 00 00 00 48 81 C4 ?? 00 00 00 41 5F 41 5E 5F";
            constexpr BYTE originalPatch[2] = { 0x32 ,0xc0 };
            constexpr const char* BlueXrayNopped = "90 90 48 8B B4 24 ?? 00 00 00 48 8B 9C 24 ?? 00 00 00 48 8B AC 24 ?? 00 00 00 48 81 C4 ?? 00 00 00 41 5F 41 5E 5F";
            constexpr BYTE noppedPatch[2] = { 0x90 ,0x90 };
            constexpr const char* ServerShotAngle = "F3 ?? ?? ?? 10 83 49 10 01 8B 41 10 F3 0F 11 41 18 83 C8 02 F3";
        }

        namespace SilentAim {

            // Shellcode。修改射击角度。该字节码写入空白区段后，读取指定位置角度达到静默自瞄目的。
            static const BYTE Shellcode[] = {
                                                                            // 保存寄存器状态
                0x50,                                                       // push rax                         保存 rax 寄存器到栈中
                0x66, 0x0F, 0xD6, 0x04, 0x24,                               // movdqa [rsp], xmm0               保存 xmm0 到栈中 (SIMD 寄存器)
                0x66, 0x0F, 0xD6, 0x4C, 0x24, 0x10,                         // movdqa [rsp+16], xmm1            保存 xmm1 到栈中 (偏移 16 字节)
                                                                            // 修改射击状态标志 (假设 rcx 指向射击数据结构)       12
                0x83, 0x49, 0x10, 0x01,                                     // or dword ptr [rcx+10], 1         设置标志位 (bit 0)
                0x8B, 0x41, 0x10,                                           // mov eax, [rcx+10]                将标志位加载到 eax
                0x83, 0xC8, 0x02,                                           // or eax, 2                        设置标志位 (bit 1)
                0x89, 0x41, 0x10,                                           // mov [rcx+10], eax                写回更新后的标志
                0x83, 0xC8, 0x04,                                           // or eax, 4                        设置标志位 (bit 2)
                                                                            // 加载并写入新的射击角度 (pitch 和 yaw)            28
                0xF3, 0x0F, 0x10, 0x05, 0x00, 0x00, 0x00, 0x00,             // movss xmm0, [rip + pitchOffset]  从内存加载 pitch 值到 xmm0
                0xF3, 0x0F, 0x10, 0x0D, 0x00, 0x00, 0x00, 0x00,             // movss xmm1, [rip + yawOffset]    从内存加载 yaw 值到 xmm1
                0xF3, 0x0F, 0x11, 0x41, 0x18,                               // movss [rcx+18], xmm0             将 pitch 值写入射击角度结构 (偏移 0x18)
                0xF3, 0x0F, 0x11, 0x49, 0x1C,                               // movss [rcx+1C], xmm1             将 yaw 值写入射击角度结构 (偏移 0x1C)
                                                                            // 恢复原始状态                                   54
                0xF3, 0x0F, 0x10, 0x47, 0x18,                               // movss xmm0, [rdi+18]             从原始数据恢复 xmm0 (假设 rdi 指向原始角度)
                0x66, 0x0F, 0xD6, 0x4C, 0x24, 0x10,                         // movdqa xmm1, [rsp+16]            从栈恢复 xmm1
                0x66, 0x0F, 0xD6, 0x04, 0x24,                               // movdqa xmm0, [rsp]               从栈恢复 xmm0
                0x58,                                                       // pop rax                          从栈恢复 rax
                                                                            // 返回原始代码流,使整体运作正常                     71
                0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, returnAddr              将返回地址加载到 rax
                0xFF, 0xE0                                                  // jmp rax                          跳转回原始代码
            };


            // JmpPatch 用于将原控制射击角度的流跳转到 shellcode位置
            static const BYTE JmpPatch[] = {
                0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, shellcodeAddr    将 shellcode 地址加载到 rax
                0xFF, 0xE0,                                                 // jmp rax                   跳转到 shellcode
                0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, // nop                       填充 nop (10 个)
                0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, // nop                       填充 nop (10 个)
                0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90        // nop                       填充 nop (9 个)
            };


            // Shellcode 中的关键偏移
            static constexpr size_t PitchOffsetPos = 32;                // pitch 偏移在 shellcode 中的位置
            static constexpr size_t YawOffsetPos = 40;                  // yaw 偏移在 shellcode 中的位置
            static constexpr size_t ReturnAddrPos = 73;                 // 返回地址在 shellcode 中的位置
            static constexpr size_t ShellcodeAddrPos = 2;               // shellcode 地址在 jmpPatch 中的位置
            static constexpr size_t ShellcodeSize = sizeof(Shellcode);  // shellcode 大小
            static constexpr size_t JmpPatchSize = sizeof(JmpPatch);    // jmpPatch 大小
            static constexpr size_t AngleOffset = 0x500;                // 射击角度偏移大小
            
        }
    }
}

#endif // CS2ASSIST_CONSTS_UTIL_H