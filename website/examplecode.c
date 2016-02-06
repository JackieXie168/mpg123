/* Leading comma means no change of previous line when appending an entry,
   comma before tab prevents uglyness because first entry would like
   alignment with a single space otherwise. */
enum some_enums
{
	SOME_CONSTANT
,	ANOTHER_CONSTANT
,	YET_ANOTHER_ONE /* No change here when extending the enum. */
}

/*
	This comment just has too
	many lines for
	me to bother aligning things with
	three spaces to match.
*/

switch(code)
{
	case SOME_CONSTANT:
		handle_const1();
	break;
	ANOTHER_CONSTANT:
	{
		int a;
		a = handle_const2();
		printf("%i\n", a);
	}
	break;
	case YET_ANOTHER_ONE:
		printf("Hurray\n");
	break;
	default:
		error("What to do?");
}

/* This is alignment, not indent, also using a leading
   comma. */
int function( int arg, char arg2
            , int arg2, char arg3 );

/* Here, things just are too long and pretty alignment is no fit. It is
   preferred to keep function attributes/type on a line with the name. */
funny_attribute_or_type_macro long function( some_type argument1
,	another_type argument2, yet_another_type argument3 );

if(TRUE) /* I do not like { at end of line. */
{
	do_something(a, b); /* Note: No space right inside parentheses. */
	/* Here, things are spacy. */
	do_more( first_argument_is_rather_long
	,	(some_condition)
		?	good_value /* two levels of indent */
		:	bad_value
	);
	a = bleh( /* Again, the ( is not separated from the function name. */
		complicated_argument
	,	function_call( some_stuff
		,	more_stuff, last_stuff )
	,	dunno_something
	); /* But the closing ) may be far away, ending the block visibly. */
} else
{
	success = -1;
	do_not_bother;
} else if(hello_there)
	nudge();

if(yes)
	return yay;
else
{
	ask_why();
	return no;
}

/* You might do pretty formatting if you want to. */
struct threesome threes[] =
{
	{ value,  "string value",               3 }
,	{ valued, "a long litany of tragedy", 123 }
,	{ v,      "short",                     12 }
};

/* Things can be combined to avoid indent cascades.
   Also, this is an example for two-character operators inside indent. */
for(i=0; i<limit; ++i) if
(
	!strcasecmp(onething, name)
||	!strcasecmp(atherthing, name)
){ /* With only ) before it, the { still is at the beginning of the line */
	do_something();
	return something;
}
