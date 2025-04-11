
cet_build_plugin(LumiStreamFilter art::module
    LIBRARIES REG
      Offline::DAQ
      Offline::RecoDataProducts
      artdaq::DAQdata
)

cet_build_plugin(Mu2ePrescaler art::module
  LIBRARIES REG
  fhiclcpp::types
  artdaq-core-mu2e::Data
  artdaq-core-mu2e::Data_dict
  artdaq::DAQdata
  otsdaq::Macros
)

install_headers(SUBDIRS detail)
install_source()
