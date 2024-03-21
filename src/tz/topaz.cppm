export module topaz;

bool initialised = false;
export namespace tz
{
	void initialise()
	{
		initialised = true;
	}	

	void terminate()
	{
		initialised = false;
	}

	bool is_initialised()
	{
		return initialised;
	}
}
