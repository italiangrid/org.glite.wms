
package org.glite.wms.wmproxy;

import org.apache.axis.MessageContext;
import org.apache.axis.encoding.DeserializationContext;
import org.apache.axis.encoding.Deserializer;
import org.apache.axis.encoding.TypeMapping;
import org.apache.axis.message.EnvelopeHandler;
import org.apache.axis.message.SOAPHandler;
import org.apache.axis.utils.XMLUtils;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

// import org.glite.ce.monitorapij.types.JobDefinition;
import org.ggf.schemas.jsdl._2005._11.jsdl.JobDefinition_Type;

import org.w3c.dom.Element;

import org.xml.sax.InputSource;

import java.io.StringReader;


/**
 * This class is a deserializer and also a serializer for the 
 * XML file containing the predefined JobDefinitions list.<br>
 * It is an extension of <code>DeserializationContext</code> axis class.
 * It is used by <code>PredefinedJobDefinitionHolder</code> to update the JobDefinition list.
 * 
 *
 * @author Luigi Zangrando (zangrando@pd.infn.it)
 */
public class JobDefinitionHandler extends DeserializationContext {
	private static Log logger = LogFactory.getLog(JobDefinitionHandler.class.getName());
	private Deserializer topDeserializer = null;

	/**
	* Creates a new JobDefinitionHandler object.<br>
	* The deserializer class for JobDefinitions is taken from context's type mapping.
	*
	* @param context The axis MessageContext.
	* @param element The <code>Element</code> relative to a JobDefinition coming from the
	* parsed XML JobDefinition list file.
	*
	* @throws Exception
	*/
	public JobDefinitionHandler (MessageContext context, Element element)throws Exception {
		super(context, new SOAPHandler());
		topDeserializer = getDeserializerForType( context.getTypeMapping().getTypeQName(JobDefinition_Type.class));
		pushElementHandler(new EnvelopeHandler((SOAPHandler) this.topDeserializer));
		inputSource = new InputSource(new StringReader(XMLUtils.ElementToString(element)));
		parse();
	}

	public JobDefinitionHandler(MessageContext context, String xml) throws Exception {
		super(context, new SOAPHandler());
		TypeMapping tm = context.getTypeMapping();
		topDeserializer =
		getDeserializerForType(context.getTypeMapping().getTypeQName(JobDefinition_Type.class));
		pushElementHandler(new EnvelopeHandler((SOAPHandler) this.topDeserializer));
		inputSource = new InputSource(new StringReader(xml));
		parse();
	}

	/**
	* Get the parsed <code>JSDL</code> casting the object obtained by the getValue() method.
	*
	* @return The parsed <code>Subscription</code>
	*/
	public JobDefinition_Type getJobDefinition() {
		Object obj = (topDeserializer == null) ? null : topDeserializer.getValue();
		return (obj == null) ? null : (org.ggf.schemas.jsdl._2005._11.jsdl.JobDefinition_Type) obj;
	}

}
