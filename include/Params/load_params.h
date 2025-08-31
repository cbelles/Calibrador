
std::string namejson;
//--CONFIG.JSON---------------------------------------------------------------
namejson = pathconfig + "/config.json";
ParamsGeneral paramsgeneral(namejson);
if (!paramsgeneral.load()) { std::cout << "Error en la apertura de config.json" << std::endl; exit(0); }
else
std::cout << "Lectura correcta de config.json" << std::endl;

//----------------------------------------------------------------------------------

//--FRUTAS.JSON---------------------------------------------------------------
namejson = pathconfig + "/frutas.json";
ParamsFruta paramsfruta(namejson);
if (!paramsfruta.load()) { std::cout << "Error en la apertura de frutas.json" << std::endl; exit(0); }
else
std::cout << "Lectura correcta de frutas.json" << std::endl;

//----------------------------------------------------------------------------------