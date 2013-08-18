ENTRY(_start)

SECTIONS
{
	. = 0x8000;
	
	.text . : ALIGN(0x20)
	{
		*(.bootup)
		__START_OF_TEXT = .;
		*(.text)
		__END_OF_TEXT = .;
	}
}

