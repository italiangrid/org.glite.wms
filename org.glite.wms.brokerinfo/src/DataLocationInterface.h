//gsoap ns service name:	DataLocationInterface
//gsoap ns service style:	rpc
//gsoap ns service encoding:    encoded 
//gsoap ns service namespace:	http://cmsdoc.cern.ch/cms/grid/docs/DataLocationInterface.wsdl
//gsoap ns service location:	http://localhost:8085

//gsoap ns schema  namespace:	urn:DataLocationInterface

/**
 *  The following file is used to generate the necessary gSOAP stubs, WSDL
 *  and other necessary files to create SOAP client-server application.
 *
 *  filename  : DataLocationInterface.h
 *  author    : Heinz Stockinger     <Heinz.Stockinger@cern.ch>
 *
 *  Copyright (c) 2004 CERN and INFN on behalf of the EU DataGrid.
 *  For license conditions see LICENSE file or
 *  http://www.edg.org/license.html
 */



class URLArray {
public:
  std::string *__ptr;
  int          __size;
  URLArray();
  void resize(int);
  std::string& operator[](int) const;
  struct soap *soap;
};


int ns__listReplicas(std::string  inputDataType,
		     std::string  inputData, 
		     URLArray    *urlList);

