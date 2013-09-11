
#pragma once

#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>

namespace CommandLine
{

class Exception
{
public:
	Exception(std::string const &parameter, char const *msg)
	{
		this->msg = parameter + ": " + msg;
	}

	char const *GetMessage() const { return msg.c_str(); }

private:
	std::string msg;
};

class Parameter
{
public:
	Parameter(char const *name, bool required, char const *helpmsg)
	{
		this->name = name;
		this->helpMessage = helpmsg;
		this->required = required;
		exists = false;
	}

	std::string const &GetName() const { return name; }
	std::string const &GetHelpMessage() const { return helpMessage; }
	std::string const &GetValue() const { return value; }
	bool Exists() const { return exists; }
	bool Required() const { return required; }

	virtual void SetValue(std::string const &value)
	{
		this->value = value;
		exists = true;
	}

private:
	std::string value;
	std::string name;
	std::string helpMessage;
	bool required;
	bool exists;

};

class Parser
{
public:
	friend class String;
	friend class Boolean;
	friend class Integer;
	friend class Real;

	static void Parse(int argc, char *argv[])
	{
		try
		{
			CreateParameterMap();
			if (!DoParse(argc, argv))
			{
				Help();
				::exit(0);
			}

		}
		catch (Exception e)
		{
			printf("%s\n", e.GetMessage());
			::exit(1);
		}
	}
	
private:
	static void CreateParameterMap()
	{
		for (auto i = parameters.begin(); i != parameters.end(); i++)
		{
			if (parameterMap.find((*i)->GetName()) != parameterMap.end()) throw Exception((*i)->GetName(), "duplicate parameter");
			parameterMap[(*i)->GetName()] = *i;
		}
	}

	static bool DoParse(int argc, char *argv[])
	{
		int i = 1;
		
		while(i < argc)
		{
			if (strlen(argv[i]) > 1 && argv[i][0] == '-')
			{
				std::string key = argv[i++] + 1;
				std::string value;

				if (key == "h" || key == "-help")
					return false;

				if (i < argc)
				{
					if (strlen(argv[i]) <= 1 || argv[i][0] != '-')
						value = argv[i++];
				}

				auto j = parameterMap.find(key);
				if (j == parameterMap.end()) throw Exception(key, "unknown parameter");
				if ((*j).second->Exists()) throw Exception(key, "parameter specified more than once");
				(*j).second->SetValue(value);
			}
			else
			{
				i++;
			}
		}

		for (auto i = parameters.begin(); i != parameters.end(); i++)
		{
			if ((*i)->Required() && !(*i)->Exists())
				throw Exception((*i)->GetName(), "parameter required");
		}

		return true;
	}

	static void AddParameter(Parameter *p)
	{
		parameters.push_back(p);
	}

	static void Help()
	{
		printf("Parameter   Description\n"); 

		for (auto i = parameters.begin(); i != parameters.end(); i++)
		{
			printf("-%-11s%s\n", (*i)->GetName().c_str(), (*i)->GetHelpMessage().c_str());
		}
	}

private:
	static std::vector<Parameter *> parameters;
	static std::map<std::string, Parameter *> parameterMap;

};

std::vector<Parameter *> Parser::parameters;
std::map<std::string, Parameter *> Parser::parameterMap;

class String : public Parameter
{
public:
	String(char const *name, bool required, char const *helpmsg)
		: Parameter(name, required, helpmsg)
	{
		Parser::AddParameter(this);
	}

	operator std::string const & () const { return GetValue(); }
	operator char const * () const { return GetValue().c_str(); }

};

class Boolean : public Parameter
{
public:
	Boolean(char const *name, bool required, char const *helpmsg)
		: Parameter(name, required, helpmsg)
	{
		Parser::AddParameter(this);
	}

	operator bool () const { return Exists(); }

};

class Integer : public Parameter
{
public:
	Integer(char const *name, bool required, char const *helpmsg, int minv, int maxv, int defv)
		: Parameter(name, required, helpmsg)
	{
		minValue = minv;
		maxValue = maxv;
		defValue = defv;
		Parser::AddParameter(this);
	}

	operator int () const { return Exists() ? ::atoi(GetValue().c_str()) : defValue; }

	virtual void SetValue(std::string const &value)
	{
		Parameter::SetValue(value);

		if (value.length() == 0) throw Exception(GetName(), "value missing");
		int v = ::atoi(value.c_str());
		if (v < minValue) throw Exception(GetName(), "value too small");
		if (v > maxValue) throw Exception(GetName(), "value too big");
	}


private:
	int minValue;
	int maxValue;
	int defValue;
};

class Real : public Parameter
{
public:
	Real(char const *name, bool required, char const *helpmsg, double minv, double maxv, double defv)
		: Parameter(name, required, helpmsg)
	{
		minValue = minv;
		maxValue = maxv;
		defValue = defv;
		Parser::AddParameter(this);
	}

	operator double () const { return Exists() ? ::atof(GetValue().c_str()) : defValue; }

	virtual void SetValue(std::string const &value)
	{
		Parameter::SetValue(value);

		if (value.length() == 0) throw Exception(GetName(), "value missing");
		double v = ::atof(value.c_str());
		if (v < minValue) throw Exception(GetName(), "value too small");
		if (v > maxValue) throw Exception(GetName(), "value too big");
	}


private:
	double minValue;
	double maxValue;
	double defValue;
};





}