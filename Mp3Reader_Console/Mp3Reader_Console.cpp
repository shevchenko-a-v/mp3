// Mp3Reader_Console.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "..\Mp3Decoder\LowLevel\Mp3Stream.h"

int main(int argc, char * argv[])
{
    if (argc == 2)
    {
        LowLevel::Mp3Stream mp3Stream(argv[1]);
        std::cout << "SUCCESS: " << mp3Stream.Parse();
    }
    std::cout << "Hello World!\n"; 
}

