

package org.glite.wms.wmproxy;
import java.io.FileReader;
import java.io.IOException;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;
import java.io.Writer;
import java.util.Calendar;
import java.util.Vector;

import javax.xml.namespace.QName;
import javax.xml.rpc.Call;
import javax.xml.rpc.ServiceException;
import javax.xml.rpc.encoding.TypeMapping;
import javax.xml.rpc.encoding.TypeMappingRegistry;

import org.apache.axis.MessageContext;
import org.apache.axis.client.AxisClient;
import org.apache.axis.client.Service;
import org.apache.axis.configuration.XMLStringProvider;
import org.apache.axis.deployment.wsdd.WSDDConstants;
import org.apache.axis.encoding.DeserializationContext;
import org.apache.axis.encoding.DeserializerFactory;
import org.apache.axis.encoding.SerializationContext;
import org.apache.axis.encoding.SerializerFactory;
import org.apache.axis.encoding.ser.BaseDeserializerFactory;
import org.apache.axis.encoding.ser.BaseSerializerFactory;
import org.apache.axis.handlers.soap.SOAPService;
import org.apache.axis.message.RPCElement;
import org.apache.axis.message.RPCParam;
import org.apache.axis.message.SOAPEnvelope;
import org.apache.axis.server.AxisServer;
import org.apache.axis.utils.XMLUtils;



import org.w3c.dom.Element;
import org.xml.sax.InputSource;

public class TestSer {

	private MessageContext msgCtx=null;
	public TestSer() throws Exception {
		Vector cachedSerQNames = new Vector();
		Vector cachedSerClasses = new Vector();
		Vector cachedSerFactories = new Vector();
		Vector cachedDeserFactories = new Vector();

		java.lang.Class cls;
		javax.xml.namespace.QName qName;
		java.lang.Class beansf = org.apache.axis.encoding.ser.BeanSerializerFactory.class;
		java.lang.Class beandf = org.apache.axis.encoding.ser.BeanDeserializerFactory.class;
		java.lang.Class enumsf = org.apache.axis.encoding.ser.EnumSerializerFactory.class;
		java.lang.Class enumdf = org.apache.axis.encoding.ser.EnumDeserializerFactory.class;
		java.lang.Class arraysf = org.apache.axis.encoding.ser.ArraySerializerFactory.class;
		java.lang.Class arraydf = org.apache.axis.encoding.ser.ArrayDeserializerFactory.class;
		java.lang.Class simplesf = org.apache.axis.encoding.ser.SimpleSerializerFactory.class;
		java.lang.Class simpledf = org.apache.axis.encoding.ser.SimpleDeserializerFactory.class;
		java.lang.Class simplelistsf = org.apache.axis.encoding.ser.SimpleListSerializerFactory.class;
		java.lang.Class simplelistdf = org.apache.axis.encoding.ser.SimpleListDeserializerFactory.class;

		qName = new javax.xml.namespace.QName("http://glite.org/wms/wmproxy", "AuthenticationFaultType");
		cachedSerQNames.add(qName);
		cls = org.glite.wms.wmproxy.AuthenticationFaultType.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://glite.org/wms/wmproxy", "AuthorizationFaultType");
		cachedSerQNames.add(qName);
		cls = org.glite.wms.wmproxy.AuthorizationFaultType.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://glite.org/wms/wmproxy", "BaseFaultType");
		cachedSerQNames.add(qName);
		cls = org.glite.wms.wmproxy.BaseFaultType.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://glite.org/wms/wmproxy", "DestURIsStructType");
		cachedSerQNames.add(qName);
		cls = org.glite.wms.wmproxy.DestURIsStructType.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://glite.org/wms/wmproxy", "DestURIStructType");
		cachedSerQNames.add(qName);
		cls = org.glite.wms.wmproxy.DestURIStructType.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://glite.org/wms/wmproxy", "GenericFaultType");
		cachedSerQNames.add(qName);
		cls = org.glite.wms.wmproxy.GenericFaultType.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://glite.org/wms/wmproxy", "GetQuotaManagementFaultType");
		cachedSerQNames.add(qName);
		cls = org.glite.wms.wmproxy.GetQuotaManagementFaultType.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://glite.org/wms/wmproxy", "GraphStructType");
		cachedSerQNames.add(qName);
		cls = org.glite.wms.wmproxy.GraphStructType.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://glite.org/wms/wmproxy", "InvalidArgumentFaultType");
		cachedSerQNames.add(qName);
		cls = org.glite.wms.wmproxy.InvalidArgumentFaultType.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://glite.org/wms/wmproxy", "JdlType");
		cachedSerQNames.add(qName);
		cls = org.glite.wms.wmproxy.JdlType.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(enumsf);
		cachedDeserFactories.add(enumdf);

		qName = new javax.xml.namespace.QName("http://glite.org/wms/wmproxy", "JobIdStructType");
		cachedSerQNames.add(qName);
		cls = org.glite.wms.wmproxy.JobIdStructType.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://glite.org/wms/wmproxy", "JobType");
		cachedSerQNames.add(qName);
		cls = org.glite.wms.wmproxy.JobType.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(enumsf);
		cachedDeserFactories.add(enumdf);

		qName = new javax.xml.namespace.QName("http://glite.org/wms/wmproxy", "JobTypeList");
		cachedSerQNames.add(qName);
		cls = org.glite.wms.wmproxy.JobTypeList.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://glite.org/wms/wmproxy", "JobUnknownFaultType");
		cachedSerQNames.add(qName);
		cls = org.glite.wms.wmproxy.JobUnknownFaultType.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://glite.org/wms/wmproxy", "NoSuitableResourcesFaultType");
		cachedSerQNames.add(qName);
		cls = org.glite.wms.wmproxy.NoSuitableResourcesFaultType.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://glite.org/wms/wmproxy", "ObjectType");
		cachedSerQNames.add(qName);
		cls = org.glite.wms.wmproxy.ObjectType.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(enumsf);
		cachedDeserFactories.add(enumdf);

		qName = new javax.xml.namespace.QName("http://glite.org/wms/wmproxy", "OperationNotAllowedFaultType");
		cachedSerQNames.add(qName);
		cls = org.glite.wms.wmproxy.OperationNotAllowedFaultType.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://glite.org/wms/wmproxy", "ProxyInfoStructType");
		cachedSerQNames.add(qName);
		cls = org.glite.wms.wmproxy.ProxyInfoStructType.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://glite.org/wms/wmproxy", "ServerOverloadedFaultType");
		cachedSerQNames.add(qName);
		cls = org.glite.wms.wmproxy.ServerOverloadedFaultType.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://glite.org/wms/wmproxy", "StringAndLongList");
		cachedSerQNames.add(qName);
		cls = org.glite.wms.wmproxy.StringAndLongList.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://glite.org/wms/wmproxy", "StringAndLongType");
		cachedSerQNames.add(qName);
		cls = org.glite.wms.wmproxy.StringAndLongType.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://glite.org/wms/wmproxy", "StringList");
		cachedSerQNames.add(qName);
		cls = org.glite.wms.wmproxy.StringList.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://glite.org/wms/wmproxy", "VOProxyInfoStructType");
		cachedSerQNames.add(qName);
		cls = org.glite.wms.wmproxy.VOProxyInfoStructType.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl-posix", "Argument_Type");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl_posix.Argument_Type.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(simplesf);
		cachedDeserFactories.add(simpledf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl-posix", "DirectoryName_Type");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl_posix.DirectoryName_Type.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(simplesf);
		cachedDeserFactories.add(simpledf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl-posix", "Environment_Type");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl_posix.Environment_Type.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(simplesf);
		cachedDeserFactories.add(simpledf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl-posix", "FileName_Type");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl_posix.FileName_Type.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(simplesf);
		cachedDeserFactories.add(simpledf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl-posix", "GroupName_Type");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl_posix.GroupName_Type.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(simplesf);
		cachedDeserFactories.add(simpledf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl-posix", "Limits_Type");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl_posix.Limits_Type.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(simplesf);
		cachedDeserFactories.add(simpledf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl-posix", "POSIXApplication_Type");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl_posix.POSIXApplication_Type.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl-posix", "UserName_Type");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl_posix.UserName_Type.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(simplesf);
		cachedDeserFactories.add(simpledf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl", "Application_Type");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl.Application_Type.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl", "Boundary_Type");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl.Boundary_Type.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(simplesf);
		cachedDeserFactories.add(simpledf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl", "CandidateHosts_Type");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl.CandidateHosts_Type.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl", "CPUArchitecture_Type");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl.CPUArchitecture_Type.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl", "CreationFlagEnumeration");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl.CreationFlagEnumeration.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(enumsf);
		cachedDeserFactories.add(enumdf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl", "DataStaging_Type");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl.DataStaging_Type.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl", "Description_Type");
		cachedSerQNames.add(qName);
		cls = java.lang.String.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(simplesf);
		cachedDeserFactories.add(simpledf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl", "Exact_Type");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl.Exact_Type.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(simplesf);
		cachedDeserFactories.add(simpledf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl", "FileSystem_Type");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl.FileSystem_Type.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl", "FileSystemTypeEnumeration");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl.FileSystemTypeEnumeration.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(enumsf);
		cachedDeserFactories.add(enumdf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl", "JobDefinition_Type");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl.JobDefinition_Type.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl", "JobDescription_Type");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl.JobDescription_Type.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl", "JobIdentification_Type");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl.JobIdentification_Type.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl", "OperatingSystem_Type");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl.OperatingSystem_Type.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl", "OperatingSystemType_Type");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl.OperatingSystemType_Type.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl", "OperatingSystemTypeEnumeration");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl.OperatingSystemTypeEnumeration.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(enumsf);
		cachedDeserFactories.add(enumdf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl", "ProcessorArchitectureEnumeration");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl.ProcessorArchitectureEnumeration.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(enumsf);
		cachedDeserFactories.add(enumdf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl", "Range_Type");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl.Range_Type.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl", "RangeValue_Type");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl.RangeValue_Type.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl", "Resources_Type");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl.Resources_Type.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://schemas.ggf.org/jsdl/2005/11/jsdl", "SourceTarget_Type");
		cachedSerQNames.add(qName);
		cls = org.ggf.schemas.jsdl._2005._11.jsdl.SourceTarget_Type.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://www.gridsite.org/namespaces/delegation-1", "DelegationExceptionType");
		cachedSerQNames.add(qName);
		cls = org.gridsite.www.namespaces.delegation_1.DelegationExceptionType.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		qName = new javax.xml.namespace.QName("http://www.gridsite.org/namespaces/delegation-1", "NewProxyReq");
		cachedSerQNames.add(qName);
		cls = org.gridsite.www.namespaces.delegation_1.NewProxyReq.class;
		cachedSerClasses.add(cls);
		cachedSerFactories.add(beansf);
		cachedDeserFactories.add(beandf);

		msgCtx = new MessageContext(new AxisClient());

		synchronized (this) {
			// must set encoding style before registering serializers
			for (int i = 0; i < cachedSerFactories.size(); ++i) {
				cls = (java.lang.Class) cachedSerClasses.get(i);
				qName = (javax.xml.namespace.QName) cachedSerQNames
					.get(i);
				java.lang.Object x = cachedSerFactories.get(i);
				if (x instanceof Class) {
				java.lang.Class sf = (java.lang.Class) cachedSerFactories.get(i);
				java.lang.Class df = (java.lang.Class) cachedDeserFactories.get(i);
				SerializerFactory   serf = BaseSerializerFactory.createFactory(sf, cls, qName);
				DeserializerFactory desf =  BaseDeserializerFactory.createFactory(df, cls, qName);
				msgCtx.getTypeMapping().register(cls, qName, serf, desf);
				} else if (x instanceof javax.xml.rpc.encoding.SerializerFactory) {
					org.apache.axis.encoding.SerializerFactory sf
						= (org.apache.axis.encoding.SerializerFactory) cachedSerFactories.get(i);
					org.apache.axis.encoding.DeserializerFactory df
						= (org.apache.axis.encoding.DeserializerFactory) cachedDeserFactories.get(i);
					msgCtx.getTypeMapping().register(cls, qName, sf, df);
				}
			}
		}
	}

	public MessageContext getMessageContext(){
		return msgCtx;
	}
}
