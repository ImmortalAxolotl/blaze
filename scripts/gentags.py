import sys
import json
import collections
import glob
import os

# Download the vanilla Minecraft server and run
#
#  java -cp server.jar net.minecraft.data.Main --all
#
# to generate all the data files into a folder called 'generated'. Provide the
# paths to the folders in 'data/minecraft/tags' to this program to convert the
# tag lists to our own (less verbose) format.

if len(sys.argv) < 2:
    print("Please specify tags folder")
    sys.exit(0)

tags = collections.OrderedDict()

for fname in glob.glob(sys.argv[1] + "/*.json"):
    f = open(fname)
    obj = json.load(f, object_pairs_hook=collections.OrderedDict)
    key = "minecraft:" + os.path.basename(fname)[:-5]
    tags[key] = obj

def print_values(key):
    for val in tags[key]["values"]:
        if val.startswith("#"):
            print_values(val[1:])
        else:
            print("value " + val)

for key in tags.keys():
    print("key " + key)
    print_values(key)
    print("")
