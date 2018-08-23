//
// Created by Peter Eichinger on 20.08.18.
//

#include "FileHelpers.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#endif


std::vector<char> readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

long getFileWriteTime(const std::string &fileName) {
#ifndef WINDOWS
    struct stat sb;
	stat(fileName.c_str(), &sb);
	return static_cast<long>(sb.st_mtime);
#else
    ULARGE_INTEGER create, access, write;
    HANDLE hFile = CreateFileA(name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    write.QuadPart = 0;
    if(hFile !=  INVALID_HANDLE_VALUE) {
        GetFileTime(hFile, LPFILETIME(&create), LPFILETIME(&access), LPFILETIME(&write));
    }
    CloseHandle(hFile);
    return write.QuadPart;
#endif

}
