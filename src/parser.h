typedef struct Parser_State
{
    Bucket_Array_Iterator current_token_it;
    Bucket_Array_Iterator peek_token_it;
} Parser_State;

Token
GetToken(Parser_State state)
{
    return *(Token*)state.current_token_it.current;
}

Token
PeekToken(Parser_State state)
{
    // NOTE: PeekToken should never be called after a Token_Invalid or Token_EndOfStream (which results in peek_token_it.current being 0)
    ASSERT(state.peek_token_it.current != 0);
    
    return *(Token*)state.peek_token_it.current;
}

enum SKIP_MODE
{
    SKIP_CURRENT,
    SKIP_PEEK,
};

void
SkipPastToken(Parser_State state, Enum8(SKIP_MODE) skip_mode)
{
    // NOTE: SkipPastToken should never be called after a Token_Invalid or Token_EndOfStream (which results in peek_token_it.current being 0)
    ASSERT(state.peek_token_it.current != 0);
    
    if (skip_mode == SKIP_CURRENT)
    {
        state.current_token_it = state.peek_token_it;
        BucketArray_AdvanceIterator(&state.peek_token_it);
    }
    
    else
    {
        BucketArray_AdvanceIterator(&state.peek_token_it);
        state.current_token_it = state.peek_token_it;
        
        if (state.peek_token_it.current != 0) BucketArray_AdvanceIterator(&state.peek_token_it);
    }
}

bool
ParseString(Compiler_State* compiler_state, String string, Memory_Arena* token_arena, Memory_Arena* string_arena, Memory_Arena* ast_arena, Error_Report* error_report)
{
    bool encountered_errors = false;
    
    Bucket_Array(Token) tokens;
    if (!LexString(string, token_arena, string_arena, &tokens, error_report))
    {
        //// ERROR: Failed to lex string
        encountered_errors = true;
    }
    
    else
    {
        
    }
    
    return !encountered_errors;
}