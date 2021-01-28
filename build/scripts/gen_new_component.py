import os
import sys
import click
import re
import shutil
import chardet

if sys.version_info[0] < 3:
    from imp import reload
    try:
        reload(sys)
        sys.setdefaultencoding('UTF8')
    except:
        pass

scriptdir = os.path.dirname(os.path.abspath(__file__))
try:
    from aos.util import locale_to_unicode
    scriptdir = locale_to_unicode(scriptdir)
except:
    pass

TEMPLATE_DIR = "templates/new_component_template"
COMPONENTS_FOLDER = "component"
AOS_MAKEFILE = "aos.mk"
AOS_MAKEFILE_TEMP = "aos.mk.temp"

def copy_file(srcfile, destfile):
    """ Copy srcfile to destfile, create destdir if not existed """
    subdir = os.path.dirname(destfile)

    if not os.path.isdir(subdir):
        os.makedirs(subdir)

    shutil.copyfile(srcfile, destfile)

def check_copy_non_utf8_file(srcfile, destfile):
    """ Copy non-utf8 srcfile to destfile, create destdir if not existed """
    non_utf8_file = False
    with open(srcfile, 'rb') as f:  
        encode = chardet.detect(f.read()) 
        if encode["encoding"] != "utf-8" and encode["encoding"] != "ascii":
            copy_file(srcfile, destfile)
            non_utf8_file = True

    return non_utf8_file

def write_file(contents, destfile):
    """ Write contents to destfile line by line """
    subdir = os.path.dirname(destfile)

    if not os.path.isdir(subdir):
        os.makedirs(subdir)
    with open(destfile, "w") as f:
        for line in contents:
            f.write(line)


def get_files_and_folder(templatedir):
    """ Get sources files and empty folder, from templatedir and subdir """
    sources = []
    for root, dirs, files in os.walk(templatedir):
        # empty folder
        if not dirs and not files:
            tempfile = root
            sources.append(tempfile.replace("\\", "/"))
        # files
        for filename in files:
            tempfile = "%s/%s" % (root, filename)
            sources.append(tempfile.replace("\\", "/"))

    templatedir = templatedir.replace("\\", "/")
    sources = [item.replace(templatedir + "/", "") for item in sources]

    return sources

def copy_template_file_or_folder(tempfile, templatedir, destdir, comptype, compname, mfname, author):
    """ Copy template file or empty folder to destdir and replace componentname with it's actual name """
    srcfile = os.path.join(templatedir, tempfile)
    contents = []

    if os.path.isdir(srcfile):
        if "componentname" in tempfile:
            tempfile = tempfile.replace("componentname", compname)
        os.makedirs(os.path.join(destdir, tempfile))
        return

    non_utf8_file = check_copy_non_utf8_file(srcfile, os.path.join(destdir, tempfile))
    if non_utf8_file:
        return

    # Replace componentname from file contents
    with open(srcfile, "r") as f:
        for line in f.readlines():
            if "@componentname@" in line:
                line = line.replace("@componentname@", compname)
            if "@COMPONENTNAME@" in line:
                line = line.replace("@COMPONENTNAME@", compname.upper().replace("-", "_"))
            if "@componenttype@" in line:
                line = line.replace("@componenttype@", comptype)
            if "@manufacturename@" in line:
                line = line.replace("@manufacturename@", mfname)
            if "@authorname@" in line:
                line = line.replace("@authorname@", author)

            contents += [line]

    # Replace componentname from filename
    if AOS_MAKEFILE_TEMP in tempfile:
        tempfile = tempfile.replace(AOS_MAKEFILE_TEMP, AOS_MAKEFILE)
    if "componentname" in tempfile:
        tempfile = tempfile.replace("componentname", compname)
    destfile = tempfile
    # Write to destfile
    if contents:
        destfile = os.path.join(destdir, destfile)
        write_file(contents, destfile)

@click.command()
@click.argument("componentname", metavar="[COMPONENTNAME]")
@click.option("-m", "--mfname", help="The manufacturer of component")
@click.option("-t", "--comptype", required=True, type=click.Choice(["bus", "dm", "fs", "gui", "language", "linkkit", "network", "peripherals", "security", "service", "utility", "wireless", "generals"]), help="The type of component")
@click.option("-a", "--author", help="The author of component")
def cli(componentname, mfname, comptype, author):
    """ Create component staging from template """
    templatedir = os.path.join(scriptdir, TEMPLATE_DIR)
    destdir = os.path.join(scriptdir, "../../", COMPONENTS_FOLDER, comptype, componentname)
    destdir = os.path.abspath(destdir)

    if os.path.isdir(destdir):
        click.echo("[Error] The component directory is existing!\n%s" % destdir)
        return 1
    else:
        os.makedirs(destdir)

    if not os.path.isdir(templatedir):
        click.echo("[Error] The component template is not evailable yet!\n")
        return 1

    sources = get_files_and_folder(templatedir)

    mfname = "manufacturername" if not mfname else mfname
    author = "authorname" if not author else author
    for tempfile in sources:
        copy_template_file_or_folder(tempfile, templatedir, destdir, comptype, componentname, mfname, author)

    click.echo("[Info] Component Initialized at: %s" % destdir)


if __name__ == "__main__":
    cli()
