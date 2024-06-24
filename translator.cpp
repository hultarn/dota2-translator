#pragma once
#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <string>
using namespace System;

namespace Dota2Translator
{
	public ref class Class1
	{
	private:
		HANDLE pHandle;
		uintptr_t baseAddr, baseAddrClient;
		uintptr_t writeChatAdd, readChatAdd, clockAdd;

	public:
		void init()
		{
			DWORD pid;
			HWND hwnd = FindWindow(nullptr, L"Dota 2");
			if (!hwnd)
			{
				Console::WriteLine("Could not find Dota 2 window.");
				return;
			}
			
			GetWindowThreadProcessId(hwnd, &pid);
			if (pid == 0)
			{
				Console::WriteLine("Could not get process ID.");
				return;
			}
			
			pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
			if (!pHandle)
			{
				Console::WriteLine("Could not open process.");
				return;
			}
			
			baseAddr = GetModuleBaseAddress(pid, L"engine2.dll");
			if (!baseAddr)
			{
				Console::WriteLine("Could not get base address of engine2.dll.");
				return;
			}
			readChatAdd = baseAddr + 0x54041A; 
			writeChatAdd = 0x19DF7C7AA60;

			baseAddrClient = GetModuleBaseAddress(pid, L"client.dll");
			if (!baseAddrClient)
			{
				Console::WriteLine("Could not get base address of client.dll.");
				return;
			}
			clockAdd = baseAddrClient + 0x29C1C18;
		}

		int readClock()
		{
			int time = 0;
			if (!ReadProcessMemory(pHandle, reinterpret_cast<LPCVOID>(clockAdd), &time, sizeof(time), nullptr))
			{
				Console::WriteLine("Failed to read clock.");
				return -1;
			}
			return time;
		}

		void writeChat(String^ textMsg)
		{
			array<Byte>^ managedArray = System::Text::Encoding::UTF8->GetBytes(textMsg);
			pin_ptr<Byte> pinnedArray = &managedArray[0];
			if (!WriteProcessMemory(pHandle, reinterpret_cast<LPVOID>(writeChatAdd), pinnedArray, managedArray->Length, nullptr))
			{
				Console::WriteLine("Failed to write chat message.");
			}
		}

		String^ readChat()
		{
			unsigned char arrText[253] = {0};
			if (!ReadProcessMemory(pHandle, reinterpret_cast<LPCVOID>(readChatAdd), arrText, sizeof(arrText), nullptr))
			{
				Console::WriteLine("Failed to read chat message.");
				return String::Empty;
			}

			return gcnew String(reinterpret_cast<char*>(arrText));
		}

	private:
		uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
		{
			uintptr_t modBaseAddr = 0;
			HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
			if (hSnap == INVALID_HANDLE_VALUE)
			{
				Console::WriteLine("Failed to create snapshot.");
				return 0;
			}
			
			MODULEENTRY32 modEntry;
			modEntry.dwSize = sizeof(modEntry);
			if (Module32First(hSnap, &modEntry))
			{
				do
				{
					if (_wcsicmp(modEntry.szModule, modName) == 0)
					{
						modBaseAddr = reinterpret_cast<uintptr_t>(modEntry.modBaseAddr);
						break;
					}
				} while (Module32Next(hSnap, &modEntry));
			}
			CloseHandle(hSnap);
			
			return modBaseAddr;
		}
	};
}