/*
 *  This file is part of RawTherapee.
 *
 *  Copyright (c) 2004-2010 Gabor Horvath <hgabor@rawtherapee.com>
 *  Copyright (c) 2010 Sasha Vasko <sasha@aftercode.net>
 *
 *  RawTherapee is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  RawTherapee is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with RawTherapee.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <safegtk.h>
#include <guiutils.h>

Glib::RefPtr<Gdk::Pixbuf> safe_create_from_file(const std::string& filename)
{
		Glib::RefPtr<Gdk::Pixbuf> res;
#ifdef GLIBMM_EXCEPTIONS_ENABLED        
    try {
        res = Gdk::Pixbuf::create_from_file (filename);
    }
    catch (Glib::Exception& ex) {
        printf ("%s\n", ex.what().c_str());
    }
#else
		std::auto_ptr<Glib::Error> error;
		res = Gdk::Pixbuf::create_from_file (filename, error);
		if (error.get())
				printf ("%s\n", error->what().c_str());
#endif
	
	return res;
}

Cairo::RefPtr<Cairo::ImageSurface> safe_create_from_png(const std::string& filename)
{
	Cairo::RefPtr<Cairo::ImageSurface> res;

  if (!Glib::file_test (filename, Glib::FILE_TEST_EXISTS))  {
  	printf ("ERROR: File \"%s\" not found.\n", filename.c_str());
  } else	{
      try {
				res = Cairo::ImageSurface::create_from_png (filename);
      } catch (...) {}
	}
	
	return res;
}

Glib::RefPtr<Gio::FileInfo> safe_query_file_info (Glib::RefPtr<Gio::File> &file)
{
		Glib::RefPtr<Gio::FileInfo> info;
#ifdef GLIBMM_EXCEPTIONS_ENABLED        
		try {	info = file->query_info();	}catch (...) {	}
#else
		std::auto_ptr<Glib::Error> error;
		info = file->query_info("*", Gio::FILE_QUERY_INFO_NONE, error);
#endif        
		return info;
}

#ifdef GLIBMM_EXCEPTIONS_ENABLED        
# define SAFE_ENUMERATOR_CODE_START \
				do{try {	if ((dirList = dir->enumerate_children ())) \
						for (Glib::RefPtr<Gio::FileInfo> info = dirList->next_file(); info; info = dirList->next_file()) {
						
# define SAFE_ENUMERATOR_CODE_END \
				}}	catch (Glib::Exception& ex) {	printf ("%s\n", ex.what().c_str());	}}while(0)
#else
# define SAFE_ENUMERATOR_CODE_START \
				do{std::auto_ptr<Glib::Error> error;	Glib::RefPtr<Gio::Cancellable> cancellable; \
					if ((dirList = dir->enumerate_children (cancellable, "*", Gio::FILE_QUERY_INFO_NONE, error))) \
						for (Glib::RefPtr<Gio::FileInfo> info = dirList->next_file(cancellable, error); !error.get() && info; info = dirList->next_file(cancellable, error)) {
						
# define SAFE_ENUMERATOR_CODE_END 	} if (error.get())	printf ("%s\n", error->what().c_str());}while (0)
#endif

void safe_build_file_list (Glib::RefPtr<Gio::File> &dir, std::vector<FileMTimeInfo> &flist)
{
    Glib::RefPtr<Gio::FileEnumerator> dirList;
    if (dir) {
				SAFE_ENUMERATOR_CODE_START
						flist.push_back (FileMTimeInfo (removeExtension(info->get_name()), info->modification_time()));
				SAFE_ENUMERATOR_CODE_END;
    }
}

void safe_build_file_list (Glib::RefPtr<Gio::File> &dir, std::vector<Glib::ustring> &names, const Glib::ustring &directory)
{
    Glib::RefPtr<Gio::FileEnumerator> dirList;
    if (dir) {
				SAFE_ENUMERATOR_CODE_START
            names.push_back (Glib::build_filename (directory, info->get_name()));
				SAFE_ENUMERATOR_CODE_END;
    }
}


void safe_build_subdir_list (Glib::RefPtr<Gio::File> &dir, std::vector<Glib::ustring> &subDirs, bool add_hidden)
{
    Glib::RefPtr<Gio::FileEnumerator> dirList;
    if (dir)
    {
        // CD-ROMs with no drive inserted are reported, but do not exist, causing RT to crash
         if (!Glib::file_test(dir->get_path(),Glib::FILE_TEST_EXISTS)) return;

				SAFE_ENUMERATOR_CODE_START
						if (info->get_file_type() == Gio::FILE_TYPE_DIRECTORY && (!info->is_hidden() || add_hidden))
								subDirs.push_back (info->get_name());
				SAFE_ENUMERATOR_CODE_END;
    }
}

Glib::ustring safe_locale_to_utf8 (const std::string& src)
{
	Glib::ustring utf8_str;
#ifdef GLIBMM_EXCEPTIONS_ENABLED
            try {
                utf8_str = Glib::locale_to_utf8(src);
            }
            catch (const Glib::ConvertError& e) {
                utf8_str = Glib::convert_with_fallback(src, "UTF8", "LATIN1","?");
            }
#else
            {
                std::auto_ptr<Glib::Error> error;
                utf8_str = locale_to_utf8(src, error);
                if (error.get())
                    utf8_str = Glib::convert_with_fallback(src, "UTF8", "LATIN1","?", error);
            }
#endif //GLIBMM_EXCEPTIONS_ENABLED
	return utf8_str;
}

std::string safe_locale_from_utf8 (const Glib::ustring& utf8_str)
{
		std::string str;
#ifdef GLIBMM_EXCEPTIONS_ENABLED
		try {
            str = Glib::locale_from_utf8(utf8_str);
        }
        catch (const Glib::ConvertError& e) {
            //str = Glib::convert_with_fallback(utf8_str, "LATIN1", "UTF8", "?");
        }
#else
        {
            std::auto_ptr<Glib::Error> error;
            str = Glib::locale_from_utf8(utf8_str, error);
            /*if (error.get())
                {str = Glib::convert_with_fallback(utf8_str, "LATIN1", "UTF8", "?", error);}*/
        }
#endif //GLIBMM_EXCEPTIONS_ENABLED
	return str;
}


bool safe_spawn_command_line_async (const Glib::ustring& cmd_utf8)
{
		std::string cmd;
		bool success = false;
#ifdef GLIBMM_EXCEPTIONS_ENABLED        
		try {
				cmd = Glib::filename_from_utf8(cmd_utf8);
				printf ("command line: |%s|\n", cmd.c_str());
				Glib::spawn_command_line_async (cmd);
				success = true;
		} catch (Glib::Exception& ex) {
				printf ("%s\n", ex.what().c_str());
		}
#else
		std::auto_ptr<Glib::Error> error;
		cmd = Glib::filename_from_utf8(cmd_utf8, error);
		if (!error.get())	{
				printf ("command line: |%s|\n", cmd.c_str());
				Glib::spawn_command_line_async (cmd, error);
			}
		if (error.get())
				printf ("%s\n", error->what().c_str());
		else
				success = true;
#endif
		return success;
}

bool safe_spawn_command_line_sync (const Glib::ustring& cmd_utf8)
{
		std::string cmd;
        std::string stdOut;
        std::string stdErr;

		bool success = false;

		int exitStatus=-1;
        try {
		    cmd = Glib::filename_from_utf8(cmd_utf8);
		    printf ("command line: |%s|\n", cmd.c_str());

            // if it crashes here on windows, make sure you have the GTK runtime files gspawn-win32-helper*.exe files in RT directory 
		    Glib::spawn_command_line_sync (cmd,NULL,NULL, &exitStatus);
        } catch (Glib::Exception& ex) {
				printf ("%s\n", ex.what().c_str());
		}
        return (exitStatus==0);
}
