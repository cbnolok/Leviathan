from os import environ

f = open("version.h", "w")

f.write("#ifndef VERSION_H \n")
f.write("#define VERSION_H \n")
f.write("#define LEVIATHAN_VERSION \\ \n")
f.write("%s\n" % environ.get("APPVEYOR_REPO_COMMIT_TIMESTAMP"))
f.write("#endif // VERSION_H \n")


f.close()
