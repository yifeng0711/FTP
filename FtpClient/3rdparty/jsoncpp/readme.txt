JSON：
	http://www.json.org/

JSONCPP
	http://sourceforge.net/projects/jsoncpp/

使用JSONCPP
	1. 反序列化：
	Json::Reader	reader; 
	Json::Value     root; 
	if(reader.parse(string, root, false))
	{
	    string cmd = root["cmd"].asString();
	}

	2. 序列化：
	Json::FastWriter	tWriter;
	Json::Value		tValue;
	tValue["ret"]		= "false";
	tValue["errorcode"] = 3001;
	tValue["errorinfo"] = "error request.";
	string tRetVal	    = tWriter.write(tValue);

	3. 注意：
	值的类型解释不一致时可能会抛出异常。