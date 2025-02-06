#define _VERSION_CODE_ "1.0.3.2"

/*
 * discordupdater
 * Copyright (C) 2025 半透明狐人間 (半狐,TranslucentFoxHuman,TlFoxHuman)
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <curl/curl.h>


/*
 * To English speakers:
 * The inserted comments are written in nonsensical English for note-taking by a Japanese speaker. Please read them patiently, and don't get angry :)
 */

std::string dlname   = "";
std::string dlfolder = "/tmp";
std::string aptinstall = "sudo apt install -y \"%s\"";
float totalsize = 0;
bool yesman = false;
bool donotoremove = false;

size_t write_data(char *buffer, size_t size, size_t nmemb, std::string gtext) {
	gtext = gtext + std::string(buffer);

	return size*nmemb;
}

size_t save_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	size_t written = fwrite(ptr, size, nmemb, stream);
	totalsize += nmemb;
	std::cout << std::fixed << std::setprecision(2);
	std::cout << "\r\033[1K" << totalsize / 1024 / 1024 << " MiB written...";
	return written;
}

std::string get_remoteversion() {
	std::string gottext;

	CURL *handle;
	CURLcode result;

	curl_global_init(CURL_GLOBAL_SSL);
	handle = curl_easy_init();

	curl_easy_setopt(handle, CURLOPT_URL, "https://discord.com/api/download/stable?platform=linux&format=deb");
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, &gottext);
	result = curl_easy_perform(handle);

	int spos = gottext.find("/linux/");
	if (spos == std::string::npos) {
		return "error";
	}
	spos += std::string("/linux/").length();
	int epos = gottext.find("/",spos);
	return gottext.substr(spos,epos-spos);
}

std::string getdownloadurl() {
	std::string gottext;

	CURL *handle;
	CURLcode result;

	curl_global_init(CURL_GLOBAL_SSL);
	handle = curl_easy_init();

	curl_easy_setopt(handle, CURLOPT_URL, "https://discord.com/api/download/stable?platform=linux&format=deb");
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, &gottext);

	result = curl_easy_perform(handle);

	int spos = gottext.find("/linux/");
	if (spos == std::string::npos) {
		return "error";
	}
	spos = gottext.find("href=\"");
	if (spos == std::string::npos) {
		return "error";
	}
	spos += std::string("href=\"").length();
	int epos = gottext.find("\"",spos);
	if (epos == std::string::npos) {
		return "error";
	}
	return gottext.substr(spos,epos-spos);
}

std::string getdlfilename() {
	std::string urldata = getdownloadurl();
	//std::cout << urldata << std::endl;
	int spos = urldata.rfind("/");
	return urldata.substr(spos + 1);
}

int download_package() {
	std::string origfname = getdlfilename();
	if (dlname == "") {
		dlname = origfname;
	}
	FILE* file = fopen((dlfolder + "/" + dlname).c_str(),"w");
	if (!file) {
		return 255;
	}
	//std::cout << dlfolder + "/" + dlname << std::endl;

	std::cout << "Downloading " + origfname + " to " + dlfolder + "/" + dlname << std::endl;

	CURL *handle;
	CURLcode result;

	curl_global_init(CURL_GLOBAL_SSL);
	handle = curl_easy_init();

	curl_easy_setopt(handle, CURLOPT_URL, getdownloadurl().c_str());
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, save_data);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, file);

	std::cout << std::defaultfloat << std::endl;

	result = curl_easy_perform(handle);
	fclose(file);
	return int(result);
}

std::string checkupdate() {
	/*  checkupdate
	 *  Returns:
	 *	  notinstalled			  : Not installed
	 *	  error:*				   : Something happened
	 *	  uptodate:(version)		: Already up-to-date
	 *	  (currentver)->(remotever) : Update available
	 */
	char buf[256];
	std::string aptres;
	std::string currentver;
	std::string remotever;
	FILE* file = popen ("LANG=C apt show discord 2>/dev/null","r");
	if (!file) {
		return "error:apt command error.";
	}
	while (fgets(buf, sizeof(buf), file) != nullptr) {
		aptres += std::string(buf);
	}
	if (aptres.find("No packages found") != std::string::npos || aptres.find("APT-Manual-Installed:") == std::string::npos) {
		return "notinstalled";
	}
	int spos = aptres.find("Version: ") + std::string("Version: ").length();
	if (spos == std::string::npos) {
		return "error:check current version error";
	}
	int epos = aptres.find("\n",spos);
	currentver = aptres.substr(spos,epos-spos);
	if (currentver == "error") {
		return "error:check available version error";
	}
	remotever = get_remoteversion();
	if(remotever != currentver) {
		return currentver + "->" + remotever;
	} else {
		return "uptodate:" + remotever;
	}
}

void command_c(void) {
	std::string vers = checkupdate();
	if (vers.find("error") !=std::string::npos) {
		std::cerr << "An error has occurred: " << vers << std::endl;
		exit(2);
	}
	if (vers == "notinstalled"){
		std::cout << "Discord is not installed." << std::endl;
		return;
	}
	int spos = vers.find("uptodate:");
	if (spos != std::string::npos)  {
		spos += std::string("uptodate:").length();
		std::cout << "Discord is already up-to-date!\nCurrent version:" + vers.substr(spos) << std::endl;
		return;
	}
	std::cout << "Update available! " << vers << std::endl;

}

int do_upgrade(void) {
	int pos = 0;
	while (pos != std::string::npos) {
		pos = aptinstall.find("%s",pos);
		if (pos == std::string::npos) {
			break;
		}
		aptinstall.replace(pos,std::string("%s").length(),dlfolder + "/" + dlname);
	}
	//std::cout << aptinstall << std::endl;
	return system(aptinstall.c_str());
}

void print_help(std::string progname) {
	std::cout << "DiscordUpdater " << _VERSION_CODE_ << "\n"
				 "Copyright 2025 TlFoxHuman(TranslucentFoxHuman, 半狐, 半透明狐人間)\n"
				 "\n"
				 "Usage:\n"
				 "" + progname + " [options]\n"
				 "\n"
				 "Options:\n"
				 "-c             : Only check updates for Discord and do not install\n"
				 "-C <cmdline>   : Specify the installation command line. \"%s\" will replaced to package name.(default: sudo apt install -y %s)\n"
				 "-d             : Only download discord.\n"
				 "-D <diractory> : Specify the download diractory.(default:/tmp)\n"
				 "-n             : Do not remove package file after installation.\n"
				 "-o <filename>  : Specify the download filename.(default is the original filename)\n"
				 "-u             : Download Discord .deb package and install it.\n"
				 "-y             : Do not ask any questions\n"
				 "-h             : Print this help.\n"
				 "-v             : Print version information." << std::endl;
}

void print_version(void) {
	std::cout << "DiscordUpdater " << _VERSION_CODE_ << "\n"
				 "Copyright 2025 TlFoxHuman(TranslucentFoxHuman, 半狐, 半透明狐人間)\n"
				 "\n"
				 "This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.\n"
				 "\n"
				 "This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.\n"
				 "\n"
				 "You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>." << std::endl;
}

int main (int argc,char **argv) {

	int commandmode = 0;

	if (argc <= 1) {
		std::string cmdline = std::string(argv[0]);
		int pos = cmdline.rfind("/");
		if (pos != std::string::npos) {
			print_help(cmdline.substr(pos));
		} else {
			print_help(cmdline);
		}
		return 0;
	}
	for (int i=1;i<argc;i++) {
		if ("-h" == std::string(argv[i])){
			std::string cmdline = std::string(argv[0]);
			int pos = cmdline.rfind("/");
			if (pos != std::string::npos) {
				print_help(cmdline.substr(pos));
			} else {
				print_help(cmdline);
			}
			return 0;
		} else if ("-v" == std::string(argv[i])) {
			print_version();
		} else if ("-y" == std::string(argv[i])) {
			yesman = true;
		} else if ("-c" == std::string(argv[i])) {
			if (commandmode != 0) {
				std::cerr << "Error! : Multiple comannd were specified." << std::endl;
				exit (10);
			}
			commandmode = 1;
		} else if ("-C" == std::string(argv[i])) {
			aptinstall = std::string(argv[i+1]);
			i++;
		} else if ("-D" == std::string(argv[i])) {
			dlfolder = std::string(argv[i+1]);
			i++;
		} else if ("-o" == std::string(argv[i])) {
			dlname = std::string(argv[i+1]);
			i++;
		} else if ("-d" == std::string(argv[i])) {
			if (commandmode != 0) {
				std::cerr << "Error! : Multiple comannd were specified." << std::endl;
				exit (10);
			}
			commandmode = 2;
		} else if ("-n" == std::string(argv[i])) {
			donotoremove = true;
		} else if ("-u" == std::string(argv[i])) {
			if (commandmode != 0) {
				std::cerr << "Error! : Multiple comannd were specified." << std::endl;
				exit (10);
			}
			commandmode = 3;
		} else {
			std::cout << "Unknown options: \"" + std::string(argv[i]) + "\"\n"
						 "Note: To reduce development effort, this program does not recognize combined options. Please describe the options separataly." << std::endl;
			return 1;
		}

	}
	switch(commandmode) {
		int res2;
		case 1:
			command_c();
		break;
		case 2:
			res2 = download_package();
			if (res2 != 0) {
				if (res2 == 255) {
					std::cerr << "Could not open the output file: " + dlfolder + "/" + dlname << std::endl;
					exit (3);
				} else {
					std::cerr << "libcurl returned error with code: " + std::to_string(res2);
					exit(res2);
				}
			}
		break;
		case 3:
			std::string vers = checkupdate();
			if (vers.find("error") !=std::string::npos) {
				std::cerr << "An error has occurred: " << vers << std::endl;
				exit(2);
			}
			int spos = vers.find("uptodate:");
			if (spos != std::string::npos)  {
				spos += std::string("uptodate:").length();
				std::cout << "Discord is already up-to-date!\nCurrent version:" + vers.substr(spos) << std::endl;
				return 0;
			}
			if (!yesman) {
				std::cout << "Update available:" + vers + "\nDo you want to install it?(Y/n): ";
				std::string yon;
				std::cin >> yon;
				if (yon == "n" || yon == "N") {
					exit(0);
				}
			} else {
				std::cout << "Update available:" + vers << std::endl;
			}
			int res = download_package();
			if (res != 0) {
				if (res == 255) {
					std::cerr << "Could not open the output file: " + dlfolder + "/" + dlname << std::endl;
					exit (3);
				} else {
					std::cerr << "libcurl returned error with code: " + std::to_string(res);
					exit(res);
				}
			}
			res = do_upgrade();
			if (!donotoremove) {
				int bres = remove(std::string(dlfolder + "/" + dlname).c_str());
				if (bres != 0) {
					std::cerr << "Could not remove package file. Package file is " + dlfolder + "/" + dlname +" . Please remove it manually." << std::endl;
				}
			}
			if (res != 0) {
				std::cerr << "Installation may failed! apt returned the code: " + std::to_string(res) << std::endl;
				exit(res);
			}
		break;

	}



	return 0;
}
