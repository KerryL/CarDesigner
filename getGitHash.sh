#!/bin/bash
# This file automatically generates a source file containing git version
# and hash information.

outputPath="$1CarDesigner/src/application/"
outputFile=vvaseConstants.cpp

gitTag=$(git describe --tags --abbrev=0)
gitHash=$(git rev-parse --short HEAD)

#echo outputPath = $outputPath
#echo outputFile = $outputFile
echo version = $gitTag
echo gitHash = $gitHash

echo // File:  $outputFile> $outputPath$outputFile
echo // Auth:  getGitHash.sh>> $outputPath$outputFile
echo // Desc:  Automatically generated file containing latest git hash and version info.>> $outputPath$outputFile
echo>> $outputPath$outputFile
echo // Local headers>> $outputPath$outputFile
echo \#include \"application/vvaseConstants.h\">> $outputPath$outputFile
echo>> $outputPath$outputFile
echo const wxString versionString = _T\(\"$gitTag\"\)\;>> $outputPath$outputFile
echo const wxString gitHash = _T\(\"$gitHash\"\)\;>> $outputPath$outputFile
