% A Libyaml Tutorial
% Andrew Poelstra
% https://www.wpsoftware.net/andrew/pages/libyaml.html

Introduction
============

This tutorial is an introduction to using the libyaml library with the C
programming language. It assumes a basic knowledge of the YAML format.
See [Wikipedia](http://en.wikipedia.org/wiki/YAML) for more information
and links to the relevant sites.

More Information
----------------

All information used to write this page came from the header
[yaml.h](./yaml.h), and trial-and-error.

YAML
====

What is YAML?
-------------

YAML stands for \`\`YAML Ain't a Markup Language'', and is a
data-storage format designed for easy readability and machine parsing.
It is more verbose than JSON, and less verbose than XML, two other
formats designed for similar purposes.

It allows data to be nested arbitrarily deep, allows embedded newlines,
and supports sequences, relational trees and all sorts of fancy stuff.
I'll be shying away from the advanced features and sticking to the
basics.

What Does it Look Like?
-----------------------

I am no YAML expert. I am learning the language as I write this — in
fact, before this morning I had never used YAML, though I'd heard of it.
Arguably, I am the last person who should be writing this tutorial. But
last I looked, there was no decent beginner's tutorial for libyaml, so
I'm doing it.

Here is the file I will be parsing:

    # config/public.yaml

    title   : Finex 2011
    img_url : /finex/html/img/
    css_url : /finex/html/style/
    js_url  : /finex/html/js/

    template_dir: html/templ/

    default_act : idx    # used for invalid/missing act=

    pages:
      - act   : idx
        title : Welcome
        html  : public/welcome.phtml
      - act   : reg
        title : Register
        html  : public/register.phtml
      - act   : log
        title : Log in
        html  : public/login.phtml
      - act   : out
        title : Log out
        html  : public/logout.phtml

One non-obvious (from this file) thing about YAML is that it forbids the
tab character. Why? Because YAML depends on indentation for its
structure, and tabs tend to mess things up.

This is part of my ongoing project to port Finex away from PHP. (Why C?
I'm trying quite a few languages, and C happens to be today's.) As you
can see, this is a simple file I'll be using to instruct my templating
system on how to read the `act=XXX` portion of a URL.

libyaml
=======

libyaml is a C library for parsing YAML files, and probably available
from your package manager. To use it, include the file `yaml.h` and add
the linker flag `-lyaml` to gcc.

For detailed information, check out the header file
`/usr/include/yaml.h`.

yaml\_parser\_t
---------------

The primary object used by libyaml is the parser itself. This is an
object of type `yaml_parser_t`. It must be allocated manually (usually
on the stack) and is initialized/deinitialized by the functions:

    int yaml_parser_initialize(yaml_parser_t *)
    void yaml_parser_delete(yaml_parser_t *)

All error codes are returned as ints. 1 signifies success, 0 failure.
Next, to open a specific file, we use the function:

    void yaml_parser_set_input_file(yaml_parser_t *parser, FILE *file)

There are also functions to read input from a string or generic read
handler, and to set the encoding of an input file. We won't cover those
here, but be aware.

Our code thus far is:

    #include <stdio.h>
    #include <yaml.h>

    int main(void)
    {
      FILE *fh = fopen("config/public.yaml", "r");
      yaml_parser_t parser;

      /* Initialize parser */
      if(!yaml_parser_initialize(&parser))
        fputs("Failed to initialize parser!\n", stderr);
      if(fh == NULL)
        fputs("Failed to open file!\n", stderr);

      /* Set input file */
      yaml_parser_set_input_file(&parser, fh);

      /* CODE HERE */

      /* Cleanup */
      yaml_parser_delete(&parser);
      fclose(fh);
      return 0;
    }

This should compile and run without error, though it doesn't do anything
yet.

Token-Based and Stream-Based Parsing
------------------------------------

There are two ways to parse a YAML document using libyaml: token-based
and event-based. The simplest way, conceptually, is token-based. By
using the functions

    int yaml_parser_scan(yaml_parser_t *parser, yaml_token_t *token)
    void yaml_token_delete(yaml_token_t *token)

we can get each token from the YAML document in turn. The full
`yaml_token_t` structure can be found in `yaml.h`, but for our purposes
we will only need the two fields `.type` and `.data.scalar.value`, which
tell us the token type, and its data (if the type is
`YAML_SCALAR_TOKEN`).

Though I do not cover every token type – again, see `yaml.h` for more –
the following example code should be illustrative:

    #include <stdio.h>
    #include <yaml.h>

    int main(void)
    {
      FILE *fh = fopen("config/public.yaml", "r");
      yaml_parser_t parser;
      yaml_token_t  token;   /* new variable */

      /* Initialize parser */
      if(!yaml_parser_initialize(&parser))
        fputs("Failed to initialize parser!\n", stderr);
      if(fh == NULL)
        fputs("Failed to open file!\n", stderr);

      /* Set input file */
      yaml_parser_set_input_file(&parser, fh);

      /* BEGIN new code */
      do {
        yaml_parser_scan(&parser, &token);
        switch(token.type)
        {
        /* Stream start/end */
        case YAML_STREAM_START_TOKEN: puts("STREAM START"); break;
        case YAML_STREAM_END_TOKEN:   puts("STREAM END");   break;
        /* Token types (read before actual token) */
        case YAML_KEY_TOKEN:   printf("(Key token)   "); break;
        case YAML_VALUE_TOKEN: printf("(Value token) "); break;
        /* Block delimeters */
        case YAML_BLOCK_SEQUENCE_START_TOKEN: puts("<b>Start Block (Sequence)</b>"); break;
        case YAML_BLOCK_ENTRY_TOKEN:          puts("<b>Start Block (Entry)</b>");    break;
        case YAML_BLOCK_END_TOKEN:            puts("<b>End block</b>");              break;
        /* Data */
        case YAML_BLOCK_MAPPING_START_TOKEN:  puts("[Block mapping]");            break;
        case YAML_SCALAR_TOKEN:  printf("scalar %s \n", token.data.scalar.value); break;
        /* Others */
        default:
          printf("Got token of type %d\n", token.type);
        }
        if(token.type != YAML_STREAM_END_TOKEN)
          yaml_token_delete(&token);
      } while(token.type != YAML_STREAM_END_TOKEN);
      yaml_token_delete(&token);
      /* END new code */

      /* Cleanup */
      yaml_parser_delete(&parser);
      fclose(fh);
      return 0;
    }

This simple loop reads every token from the document and prints it out.
The output for `public.yaml`, indented to show block structure, is:

    STREAM START  
      [Block mapping] 
      (Key token)   scalar title 
      (Value token) scalar Finex 2011 
      (Key token)   scalar img_url 
      (Value token) scalar /finex/html/img/ 
      (Key token)   scalar css_url 
      (Value token) scalar /finex/html/style/ 
      (Key token)   scalar js_url 
      (Value token) scalar /finex/html/js/ 
      (Key token)   scalar template_dir 
      (Value token) scalar html/templ/ 
      (Key token)   scalar pages 
      (Value token) Start Block (Sequence)
        Start Block (Entry)
          [Block mapping] 
          (Key token)   scalar act 
          (Value token) scalar idx 
          (Key token)   scalar title 
          (Value token) scalar Welcome 
          (Key token)   scalar html 
          (Value token) scalar public/welcome.phtml 
        End block
        Start Block (Entry)
          [Block mapping] 
          (Key token)   scalar act 
          (Value token) scalar reg 
          (Key token)   scalar title 
          (Value token) scalar Register 
          (Key token)   scalar html 
          (Value token) scalar public/register.phtml 
        End block
        Start Block (Entry)
          [Block mapping] 
          (Key token)   scalar act 
          (Value token) scalar log 
          (Key token)   scalar title 
          (Value token) scalar Log in 
          (Key token)   scalar html 
          (Value token) scalar public/login.phtml 
        End block
        Start Block (Entry)
          [Block mapping] 
          (Key token)   scalar act 
          (Value token) scalar out 
          (Key token)   scalar title 
          (Value token) scalar Log out 
          (Key token)   scalar html 
          (Value token) scalar public/logout.phtml 
        End block
      End block
    End block
    STREAM END  

It is clear that for simple documents, token-based parsing makes sense.
However, a more natural paradigm is event-based parsing. This works by
the similar functions

    int yaml_parser_parse(yaml_parser_t *parser, yaml_event_t *event)
    void yaml_event_delete(yaml_event_t *event)

The code using these functions, and its output, is as follows:

    #include <stdio.h>
    #include <yaml.h>

    int main(void)
    {
      FILE *fh = fopen("config/public.yaml", "r");
      yaml_parser_t parser;
      yaml_event_t  event;   /* New variable */

      /* Initialize parser */
      if(!yaml_parser_initialize(&parser))
        fputs("Failed to initialize parser!\n", stderr);
      if(fh == NULL)
        fputs("Failed to open file!\n", stderr);

      /* Set input file */
      yaml_parser_set_input_file(&parser, fh);

      /* START new code */
      do {
        if (!yaml_parser_parse(&parser, &event)) {
           printf("Parser error %d\n", parser.error);
           exit(EXIT_FAILURE);
        }

        switch(event.type)
        { 
        case YAML_NO_EVENT: puts("No event!"); break;
        /* Stream start/end */
        case YAML_STREAM_START_EVENT: puts("STREAM START"); break;
        case YAML_STREAM_END_EVENT:   puts("STREAM END");   break;
        /* Block delimeters */
        case YAML_DOCUMENT_START_EVENT: puts("<b>Start Document</b>"); break;
        case YAML_DOCUMENT_END_EVENT:   puts("<b>End Document</b>");   break;
        case YAML_SEQUENCE_START_EVENT: puts("<b>Start Sequence</b>"); break;
        case YAML_SEQUENCE_END_EVENT:   puts("<b>End Sequence</b>");   break;
        case YAML_MAPPING_START_EVENT:  puts("<b>Start Mapping</b>");  break;
        case YAML_MAPPING_END_EVENT:    puts("<b>End Mapping</b>");    break;
        /* Data */
        case YAML_ALIAS_EVENT:  printf("Got alias (anchor %s)\n", event.data.alias.anchor); break;
        case YAML_SCALAR_EVENT: printf("Got scalar (value %s)\n", event.data.scalar.value); break;
        }
        if(event.type != YAML_STREAM_END_EVENT)
          yaml_event_delete(&event);
      } while(event.type != YAML_STREAM_END_EVENT);
      yaml_event_delete(&event);
      /* END new code */

      /* Cleanup */
      yaml_parser_delete(&parser);
      fclose(fh);
      return 0;
    }

    STREAM START 
    Start Document 
      Start Mapping 
        Got scalar (value title) 
        Got scalar (value Finex 2011) 
        Got scalar (value img_url) 
        Got scalar (value /finex/html/img/) 
        Got scalar (value css_url) 
        Got scalar (value /finex/html/style/) 
        Got scalar (value js_url) 
        Got scalar (value /finex/html/js/) 
        Got scalar (value template_dir) 
        Got scalar (value html/templ/) 
        Got scalar (value pages) 
        Start Sequence 
          Start Mapping 
            Got scalar (value act) 
            Got scalar (value idx) 
            Got scalar (value title) 
            Got scalar (value Welcome) 
            Got scalar (value html) 
            Got scalar (value public/welcome.phtml) 
          End Mapping 
          Start Mapping 
            Got scalar (value act) 
            Got scalar (value reg) 
            Got scalar (value title) 
            Got scalar (value Register) 
            Got scalar (value html) 
            Got scalar (value public/register.phtml) 
          End Mapping 
          Start Mapping 
            Got scalar (value act) 
            Got scalar (value log) 
            Got scalar (value title) 
            Got scalar (value Log in) 
            Got scalar (value html) 
            Got scalar (value public/login.phtml) 
          End Mapping 
          Start Mapping 
            Got scalar (value act) 
            Got scalar (value out) 
            Got scalar (value title) 
            Got scalar (value Log out) 
            Got scalar (value html) 
            Got scalar (value public/logout.phtml) 
          End Mapping 
        End Sequence 
      End Mapping 
    End Document 
    STREAM END 

There are two major things to notice about this. First, the output is
better-structured; the generic \`\`End Block'' tokens are now specific
events. Secondly, the code is simpler. Not only that, but my `switch`
has *every* event type. As you may recall from the token-based code, the
`switch` statement was incomplete, and should have contained many other
token types, most of which I don't understand.

Also, an event-based approach is more amenable to object-oriented
programming, and so is likely what you'll see when using libyaml with
other languages.

Document-Based Parsing
----------------------

I lied. There is actually a third way to parse YAML, based on the
functions

    int yaml_parser_load(yaml_parser_t *parser, yaml_document_t *document)
    void yaml_document_delete(yaml_document_t *document)

These allow you to load individual documents into structures, and
manipulate them using a variety of `yaml_document_*` functions. This is
useful because YAML documents may be spread across multiple files, or
individual files may contain many YAML documents. However, this is an
advanced use case and I haven't looked into it.

.

Looking Forward
===============

This concludes my tutorial. Looking forward, there are a few things to
look out for. First, watch out for memory leaks. While YAML does not
allocate and return complete objects, any function that populates an
existing object will allocate buffers. To avoid leaking, be sure to use
the appropriate `yaml_*_delete` function before re-populating an object.

Also, watch out for versioning. The current YAML version is 1.3, but the
libyaml version I'm using only supports up to 1.2. If this is a concern
for you, you can use the functions `yaml_get_version` and
`yaml_get_version_string` to see what your library supports.

Watch out for encoding issues. This is the 21st century, after all. The
`STREAM_START` event and token both have an `.encoding` field that you
can check if you do not know your document encoding in advance.

Finally, you can build YAML trees in code. There are a variety of
functions to create your own tokens, events and documents, and the
`yaml_emitter_t` object (and associated functions) will allow you to
output files. See `yaml.h` for details.

January 2011\
*Andrew Poelstra*
