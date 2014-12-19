# encoding utf8
import cloud

def svr_ping():
	return "hello"

if __name__=="__main__":
	protocals = (
		{
			"label": "ping",
			"func": svr_ping,
			"out_encoding": "raw", 
		},
	)
	url_map = {}
	for pro in protocals:
		url = cloud.rest.publish(**pro)
		url_map[pro["label"]]=url.encode()
	f = open("protocal.json","w")
	f.writelines("{\n")
	for name, url in url_map.iteritems():
		f.writelines("\t\"%s\" : \"%s\",\n"%(name,url))
	f.writelines("}\n")
	f.close()

