#ifndef STRINGS_H
#define STRINGS_H

static inline u32 getStringLength(char* str)
{
    u32 length = 0;
    
    while(str && str[length] != 0)
    {
        ++length;
    }
    
    return length;
}

static char* duplicateString(char* str)
{
    if (!str || str[0] == 0)
        return 0;
    
    u32 strLength = getStringLength(str);
    char* result = (char*)allocMemory(strLength + 1);
    
    for (u32 i = 0; str[i] != 0; ++i)
    {
        result[i] = str[i];
    }
    
    return result;
}

// returns a newly allocated string that is the concatenation of str1 & str2
static char* concatStrings(char* str1, char* str2)
{
    if (!str1 || str1[0] == 0)
        return duplicateString(str2);
    if (!str2 || str2[0] == 0)
        return duplicateString(str1);
    
    u32 str1Length = getStringLength(str1);
    u32 str2Length = getStringLength(str2);
    
    u32 index = 0;
    char* result = (char*)allocMemory(str1Length + str2Length + 1);
    
    while (index < str1Length)
    {
        result[index] = str1[index];
        ++index;
    }
    
    while (index - str1Length < str2Length)
    {
        result[index] = str2[index - str1Length];
        ++index;
    }
    
    return result;
}

static bool stringEndsWith(char* string, char* substring)
{
    bool result = true;
    
    if (!string || !substring || string[0] == 0 || substring[0] == 0)
        return false;
    
    u32 length = getStringLength(string);
    u32 substringLength = getStringLength(substring);
    
    if (substringLength > length)
        return false;
    
    u32 stringIndex = length - substringLength;
    u32 substringIndex = 0;
    
    while (stringIndex < length && substringIndex < substringLength &&
           string[stringIndex] == substring[substringIndex])
    {
        ++stringIndex;
        ++substringIndex;
    }
    
    if (stringIndex < length || substringIndex < substringLength)
        result = false;
    
    return result;
}

#endif //STRINGS_H
