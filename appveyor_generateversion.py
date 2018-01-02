# This script overwrites the standard version.h, which is used for non-automatic builds

from os import environ

f = open("src/version.h", "w")

f.write("#ifndef VERSION_H\n")
f.write("#define VERSION_H\n")
f.write("#define LEVIATHAN_VERSION \\\n")
# date is in ISO 8601 format (like 2016-02-18T15:18:39.0000000Z), where Z is the time zone
if ("APPVEYOR_REPO_TAG_NAME" in environ):
	f.write("%s - %s\n" % (environ.get("APPVEYOR_REPO_TAG_NAME"), environ.get("APPVEYOR_REPO_COMMIT_TIMESTAMP")))
else:
	f.write("%s\n" % environ.get("APPVEYOR_REPO_COMMIT_TIMESTAMP"))
f.write("#endif // VERSION_H\n")

f.close()
