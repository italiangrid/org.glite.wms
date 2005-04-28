#ifndef GLITE_WMS_CLIENT_ADUTILS
#define GLITE_WMS_CLIENT_ADUTILS
/*
 * adutils.h
 */
 namespace glite { namespace wms { namespace jdl {
 	class Ad ;
	class JobAd;
	class ExpDagAd;
 } } }
void setDefaultValuesAd(glite::wms::jdl::Ad* jdl,	glite::wms::jdl::Ad& conf);
void setDefaultValues(glite::wms::jdl::JobAd* jdl,	glite::wms::jdl::Ad& conf);
void setDefaultValues(glite::wms::jdl::ExpDagAd* jdl,	glite::wms::jdl::Ad& conf);
#endif
