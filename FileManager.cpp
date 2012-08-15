#include "FileManager.h"
#include "StringUtils.h"

#include <windows.h>
#include <sstream>

#define FindNextFile FindNextFileA
#define FindFirstFile FindFirstFileA
#define WIN32_FIND_DATA WIN32_FIND_DATAA


//////////////////////////////////////////////////////////////////////////////
// Fonctions priv�es
//////////////////////////////////////////////////////////////////////////////

ProtoDirContent *FileManager::GetDirectoryContent(string _dirPath)
{
	cout << "Target directory is " << _dirPath.c_str() << endl;

	// Initialisation du vecteur � retourner
	ProtoDirContent* fileList = new ProtoDirContent();
	
	// Pr�paration de la chaine pour l'utilisation de la fonction FindFile
	// On ajoute "\\*" � la fin du nom de repertoire.
	_dirPath += "\\*";
	
	// On v�rifie que le chemin ne soit pas plus grand que la taille maximum autoris�e (MAX_PATH) 
	if (_dirPath.length() > MAX_PATH) {
		cout << "Directory path is too long." << endl;
		return fileList;
	}

	// Recherche du premier fichier dans le repertoire.
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;
	//LPCWSTR str = StringUtils::StringToBStr(_dirPath);
	hFind = FindFirstFile(_dirPath.c_str(), &ffd);
	if (hFind == INVALID_HANDLE_VALUE) {
		cout << "FindFirstFile error : INVALID_HANDLE_VALUE" << endl;
		return fileList;
	} 
   
	// Lister tous les fichiers du repertoire en r�cup�rant quelques infos.
	LARGE_INTEGER filesize;
	ostringstream data;
	ProtoFile *file;
	do {

		data.str("");
		data << ffd.cFileName;

		// Si c'est un r�pertoire
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			
			// skip le r�pertoire "." et "$RECYCLE.BIN"
			if (strcmp(ffd.cFileName, ".") == 0 || strcmp(ffd.cFileName, "$RECYCLE.BIN") == 0 ) {
				continue;
			}
			
			file = fileList->add_file();
			file->set_path(_dirPath);
			file->set_name(data.str());
			file->set_type(ProtoFile_FileType_DIRECTORY);
			
			filesize.LowPart = ffd.nFileSizeLow;
			filesize.HighPart = ffd.nFileSizeHigh;
			//data << "<" << filesize.QuadPart << " bytes>";
			file->set_size((int) filesize.QuadPart);

			wcout << ffd.cFileName << " <DIR>" << endl;


		// Si c'est un fichier
		} else {
			
			file = fileList->add_file();
			file->set_path(_dirPath);
			file->set_name(data.str());
			file->set_type(ProtoFile_FileType_FILE);

			filesize.LowPart = ffd.nFileSizeLow;
			filesize.HighPart = ffd.nFileSizeHigh;
			//data << "<" << filesize.QuadPart << " bytes>";
			file->set_size((int) filesize.QuadPart);

			wcout << ffd.cFileName << " " << filesize.QuadPart << " bytes" << endl;

		}
	} while (FindNextFile(hFind, &ffd) != 0);
 
	dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES) {
		cout << "FindFirstFile error : " << dwError << endl;
		return fileList;
	}

	FindClose(hFind);

	return fileList;
}

//////////////////////////////////////////////////////////////////////////////
// Fonctions publics
//////////////////////////////////////////////////////////////////////////////

/*
string FileManager::ListFilesStr(string _dirPath)
{
	vector<string> fileList = ListFiles(_dirPath);
	const int listSize = fileList.size();

	string result = "";
	for(int i = 0; i < listSize; i++) {
		result.append(fileList[i]);
		if (i != listSize - 1) {
			result.append("|");
		}
	}
	return result;
}
*/

string FileManager::OpenFile(string _filePath)
{
	bstr_t filePath(_filePath.c_str());
	ShellExecute(NULL, NULL, filePath, NULL, NULL, SW_SHOWMAXIMIZED);
	return "Ouverture du fichier : "  + _filePath;
}