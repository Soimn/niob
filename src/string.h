bool
StringCompare(String s0, String s1)
{
    while (s0.size && s1.size && *s0.data == *s1.data)
    {
        ++s0.data;
        --s0.size;
        ++s1.data;
        --s1.size;
    }
    
    return (s0.size == 0 && s1.size == 0);
}

String
DirectoryName(String path)
{
    String name = {
        .data = path.data,
        .size = 0
    };
    
    for (umm i = 0; i < path.size; ++i)
    {
        if (i != path.size - 1 && path.data[i] == '/')
        {
            name.data = path.data + (i + 1);
            name.size = 0;
        }
        
        else name.size += 1;
    }
    
    return name;
}