#include "openssl/sha.h"

#include <string>
#include <sstream>
#include <iomanip>

std::string hash_to_hex (const std::string& hash)
{
    std::stringstream outstream;
//    outstream << std::setfill('0') << std::setw(2) << std::hex;
    assert(hash.length() == SHA256_DIGEST_LENGTH);
    for (uint8_t i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        outstream << std::setw(2) << std::setfill('0') << std::hex << (unsigned int)(uint8_t)hash[i];
        //sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }

    //outputBuffer[64] = 0;
    return outstream.str();
}


std::string sha256_string(std::string str)
{
//    std::stringstream outstream;
//    outstream << std::setfill('0') << std::setw(2) << std::hex;

    str.reserve(SHA256_DIGEST_LENGTH * 2 + 1);
    //unsigned char hash[SHA256_DIGEST_LENGTH];
    std::string hash(SHA256_DIGEST_LENGTH, '\000');

    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.length());
    SHA256_Final((unsigned char *) hash.c_str(), &sha256);
//    for(uint8_t i = 0; i < SHA256_DIGEST_LENGTH; i++)
//    {
//        //sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
//        outstream << hash[i];
//    }
    //outputBuffer[64] = 0;
    return hash;
}

std::string sha256_file(char *path)
{
    FILE *file = fopen(path, "rb");
    if (!file) throw std::runtime_error("File can not be opened");

    std::string hash(SHA256_DIGEST_LENGTH, '\000');

    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    const int bufSize = 32768;
    unsigned char *buffer = new unsigned char [bufSize];
    int bytesRead = 0;
    while((bytesRead = fread(buffer, 1, bufSize, file)))
    {
        SHA256_Update(&sha256, buffer, bytesRead);
    }
    SHA256_Final((unsigned char *) hash.c_str(), &sha256);

    //auto res = hash_to_hex(hash);
    fclose(file);
    delete [] buffer;

    return hash;
}