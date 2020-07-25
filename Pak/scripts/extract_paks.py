import os
import shutil
import glob
import sys
import subprocess

if len(sys.argv) < 3:
    print('Usage: python extract_paks.py "Path\\To\\MXM" "Path\\To\\Extraction"')
    print('Example: python extract_paks.py "C:\\MXM" "C:\\MXM_extracted"')
    exit()

script_dir = os.path.dirname(os.path.realpath(__file__))
cur_dir = os.path.realpath(os.getcwd())
input_dir = sys.argv[1]
output_dir = sys.argv[2]

#print(script_dir, cur_dir, input_dir, output_dir)

tool_path = os.path.realpath(os.path.join(script_dir, "pak.exe"))
#print(tool_path)

pak_files = glob.glob(input_dir + '/**/*.pak', recursive=True)
#print(pak_files)

for f in pak_files:
    fdir = os.path.dirname(f)[len(input_dir)+1:]
    filename = os.path.basename(f)
    
    #print(filename)
    #print(filename[:-4])
    #print(output_dir)
    #print(fdir)
    os.makedirs(os.path.join(output_dir, fdir), exist_ok=True)

    print('"%s" "%s" "%s"' % (tool_path, f, os.path.join(output_dir, fdir, filename[:-4])))
    #os.system('"%s" "%s" "%s"' % (tool_path, f, os.path.join(output_dir, fdir, filename[:-4])))
    subprocess.run([tool_path, f, os.path.join(output_dir, fdir, filename[:-4])])