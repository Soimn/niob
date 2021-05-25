#include "niob.h"
#include "niob.c"

int
main(int argc, const char** argv)
{
    DEBUG_PrintTokens(CONST_STRING("void\n"
                                   "BucketArray_AdvanceIterator(Bucket_Array* array, Bucket_Array_Iterator* it)\n"
                                   "{0.00005e+100\n"
                                   "ASSERT(it->current != 0);\n    "
                                   "                                     \n                "
                                   "it->index += 1;     \n"
                                   "umm offset = it->index % array->bucket_size;\n "
                                   "\n"
                                   "if (*(void**)it->current_bucket != 0 || offset < array->current_bucket_size)\n"
                                   "{\n"
                                   "if (offset == 0) it->current_bucket = *(void**)it->current_bucket;\n"
                                   "\n"
                                   "it->current = (u8*)it->current_bucket + sizeof(u64) + offset * array->element_size;\n"
                                   "}\n"
                                   "}\n\0"));
}