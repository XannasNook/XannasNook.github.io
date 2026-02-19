/// Moves to the next instruction in the current open script
bool ScriptNext()
{
	assert(_nameText && "ERROR: ScriptNext: No name text box set!");
	assert(_dialogueText && "ERROR: ScriptNext: No dialogue text box set!");

    // If the script is open, we are not in a choice, and we are not in a fade transition
	if (!scriptPointer || inChoice || fading) return false 
	
    // get a pointer to the current line of the script
	linePtr = ftell(scriptPointer);												
	// create a buffer
    char line[256];																		
    // get the first line of the script and put it in the buffer
	fgets(line, 256, scriptPointer);								
    // if the line exists
	if (!line) return false;																			
				
    // loop over any blank lines
	while (!strcmp(line, "\n") || !strncmp(line, "//", 2))					
		fgets(line, 256, scriptPointer);

    // this variable will hold the context of the next tokens to be parsed
	char* next_token = NULL;							
    // this function allows you to loop through the tokens (words) of a 
    // string (in this case we use it to modify line and remove any new lines in it)
	char* token = strtok_s(line, "\n", &next_token);		
	char toUse[256];										
    // create a copy of the line buffer that will be modified
	CopyString(toUse, line);						

    // get the first token of the modifiable buffer
	token = strtok_s(toUse, " ", &next_token);				
	
    // CHECKING FOR COMMANDS -----------------------------------------
  
    if (TokenIsString("Char:"))
	{
		token = strtok_s(NULL, " ", &next_token);
		char name[32];
		if (TokenIsString("#PC"))
			strcpy_s(name, 32, pcName);
		else
			strcpy_s(name, 32, token);
		ScriptSetCharacter(name);
		ScriptNext();
	}
	else if (TokenIsString("Background:"))
	{
		token = strtok_s(NULL, " ", &next_token);
		
		ScriptSetBackground(token);
		ScriptNext();
	}
	else if (TokenIsString("FADE:"))
	{
        // get the next line
		fgets(line, 256, scriptPointer);	
        // remove all tab and new line
		char none = *strtok_s(line, "\t\n", &next_token);	
		TextSetText(fadeText, line);
		TimerReset(fadeTimer);
		TimerStart(fadeTimer);
		fading = true;
	}
	else if (TokenIsString("Button" ))
	{
		ScriptReadButton(token, line, next_token, toUse);
	}
	else if (TokenIsString("Item" ))
	{
		ScriptReadItem(token, next_token, line, toUse);
	}
	else if (TokenIsString("jmp" ))
	{
		token = strtok_s(NULL, " ", &next_token);
		ScriptJumpTo(token);
		ScriptNext();
	}
	else if (TokenIsString("fileJmp" ))
	{
		token = strtok_s(NULL, " ", &next_token);
		ScriptOpen(token);
		ScriptNext();
	}
	else if (TokenIsString("return" ))
	{
		ScriptReturn();
		ScriptNext();
	}
	else if (TokenIsString("Label" ))
	{
		ScriptNext();
	}
	else if (TokenIsString("end" ))	// if we reach the end of file			
	{
		TextSetText(_nameText, "");
		TextSetText(_dialogueText, "end");
		ScriptFree();
	}
	else // if there is no instruction, it is a line of dialogue																			
	{
		char* parsedLine = calloc(256, sizeof(char));
		ScriptParseInline(line, parsedLine);
		TextSetText(_dialogueText, parsedLine);
		char textLogLine[256];
		sprintf_s(textLogLine, 256, "%s:\n%s", curCharacter, parsedLine);
		TextLogSystemAddLine(textLogLine);
		free(parsedLine);
	}
	return true;
}