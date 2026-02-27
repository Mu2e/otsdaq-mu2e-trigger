
include(otsdaq::table)

cet_build_plugin(TriggerConfigTable otsdaq::table LIBRARIES REG
  )


cet_build_plugin(DBServiceTable otsdaq::table LIBRARIES REG
  )

install_headers()
install_source()
