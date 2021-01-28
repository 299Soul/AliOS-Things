import sys, os

component_mk_str = "#warning: aos auto component, don't change everything here.\nNAME := auto_component\n$(NAME)_MBINS_TYPE := share"

if len(sys.argv) == 2:
   file_dir = os.path.join(os.getcwd(), sys.argv[1])
   if not os.path.exists(file_dir):
       os.makedirs(file_dir)
   with open(os.path.join(file_dir, "aos.mk"), "w") as f:
       f.write(component_mk_str)
