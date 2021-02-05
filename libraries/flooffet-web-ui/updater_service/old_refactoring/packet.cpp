//need separate source for this
struct packet {
	string buffer;
	
	packet& operator + (const packet&);
	packet& operator + (const string&);
	
	//implement these somewhere else
	void send();
	void send_all(){};
	void get(uint32_t sender);
	
	packet(string buf = "")
	{
		buffer = buf;
	}
	
private:
	string next_value(size_t &);
};
	
packet& packet::operator + (const packet &b)
{
	this->buffer += b.buffer;
	return *this;
}

packet& packet::operator + (const string &b)
{
	this->buffer += b;
	return *this;
}
