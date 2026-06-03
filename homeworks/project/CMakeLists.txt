cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(GAMES102_Project VERSION 0.0.2)
message(STATUS "[Project] ${PROJECT_NAME}")

include(cmake/InitUCMake.cmake)
Ubpa_InitUCMake()

Ubpa_InitProject()

set(Ubpa_USRefl_Build_AutoRefl TRUE CACHE BOOL "use auto refl" FORCE)

Ubpa_AddDep(Utopia 0.0.5)
Ubpa_AddDep(UHEMesh 0.5.3)

Ubpa_DownloadZip(
  file://E://VScpp//git//games102New//assets_v2.zip
  assets_v2.zip
  SHA256 88004765E50821A1C77958294A11EBF5AE28D5F2D5BF523B2B47C06FB7BDDBDD
)

Ubpa_AddSubDirsRec(src)
