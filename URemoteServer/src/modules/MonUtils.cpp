#include "MonUtils.h"
#include <iostream>

#define ERROR_INDEX -1
#define FIRST_MON_ID 0

// Initialisation du vecteur
std::vector<HMONITOR> MonUtils::s_vcMonitors = std::vector<HMONITOR>();

/**
 * CALLBACK permettant de lister les moniteurs
 */
BOOL CALLBACK MonUtils::MonitorEnumProc(HMONITOR _monitor, HDC _hdcMonitor, LPRECT _lpMonitorRect, LPARAM _dwData)
{	
	s_vcMonitors.push_back(_monitor);
	return TRUE;
}

/**
 * @param _monitor Le moniteur dont on veut connaitre l'index
 * @return L'index du moniteur courant.
 * Retourne -1 en cas d'erreur.
 */
int MonUtils::GetMonitorIndex(HMONITOR _monitor)
{
	const int monitorsCount = s_vcMonitors.size();
	for (int i= 0; i < monitorsCount; i++) {
		if (s_vcMonitors[i] == _monitor)
			return i;
	}

	return ERROR_INDEX;
}

/**
 * @param _monitor Le moniteur pr�c�dent celui dont on veut connaitre l'index.
 * @return L'index du moniteur suivant. 
 * Si on est sur le dernier moniteur, on boucle sur le premier.
 * Retourne -1 en cas d'erreur.
 */
int MonUtils::GetNextMonitorIndex(HMONITOR _monitor)
{
	const int monitorsCount = GetSystemMetrics(SM_CMONITORS);
	const int currentIndex = GetMonitorIndex(_monitor);

	// Si GetMonitorIndex a renvoy� une erreur
	if (currentIndex == ERROR_INDEX) {
		return ERROR_INDEX;

	// Si Le moniteur n'est pas le dernier moniteur de la liste
	} else if (currentIndex < monitorsCount -1) {
		return currentIndex + 1;
	}
	
	// Le moniteur est le dernier, on boucle sur le premier
	return FIRST_MON_ID;
}

/**
 * @param _monitor Le moniteur pr�c�dent celui que l'on veut r�cup�rer.
 * @return Le moniteur suivant celui pass� en param�tre.
 */
HMONITOR MonUtils::GetNextMonitor(HMONITOR _monitor)
{
	int nextMonitorIndex = GetNextMonitorIndex(_monitor);

	if (nextMonitorIndex == ERROR_INDEX)
		return nullptr;
	
	return s_vcMonitors[nextMonitorIndex];
}

/**
 * @param _monitor Le moniteur dont on veut connaitre les dimensions.
 * @return Le rectangle d�limitant la position et les dimensions du moniteur.
 * Retourne nullptr en cas d'erreur.
 */
Rect* MonUtils::GetMonitorRect(HMONITOR _monitor)
{
	MONITORINFO monitorInfo;
	monitorInfo.cbSize = sizeof(monitorInfo);
	
	if (GetMonitorInfo(_monitor, &monitorInfo) != 0) {
		return new Rect(monitorInfo.rcMonitor);
	}
	
	return nullptr;
}

/**
 * @param _window La fen�tre dont on veut connaitre la position relative.
 * @param _rectMon La position du moniteur.
 * @return La position relative de la fen�tre par rappot au moniteur.
 * Retourne nullptr en cas d'erreur.
 */
Rect* MonUtils::GetRelativePos(HWND _window, Rect _rectMon)
{
	// R�cuperation de la position de la fen�tre
	RECT windowPos;
	if ( GetWindowRect(_window, &windowPos)  !=0 ) {
		Rect *relativePos = new Rect();
		// Calcul de l'offset en pixels
		relativePos->left	= windowPos.left	- _rectMon.left;
		relativePos->top	= windowPos.top		- _rectMon.top;
		relativePos->right	= windowPos.right	- _rectMon.left;
		relativePos->bottom	= windowPos.bottom	- _rectMon.top;
		return relativePos;
	}

	return nullptr;
}

/**
 * Permet de calculer les coefficients de redimensionnement en largeur et en hauteur entre deux rectangles donn�s
 * @param _rec1 Le rectangle 1
 * @param _rec2 Le rectangle 2
 * @return Le coefficient de transition entre les deux rectangles.
 */
PointF MonUtils::GetRectCoef(Rect _rect1, Rect _rect2)
{
	PointF coef;
	coef.x = (float) (_rect2.GetWidth()) / (float) (_rect1.GetWidth());
	coef.y = (float) (_rect2.GetHeight()) / (float) (_rect1.GetHeight());
	
	return coef;
}

/**
 * Application d'un coefficient � un rectange.
 * @param rectIn Le rectangle � redimensionner.
 * @param coef Le coefficient � appliquer.
 * @return Le rectangle redimensionn�.
 */
Rect* MonUtils::ApplyCoef(Rect rectIn, PointF coef)
{
	Rect *rectOut = new Rect();
	rectOut->left	= (int) ((float)rectIn.left		* coef.x);
	rectOut->top	= (int) ((float)rectIn.top		* coef.y);
	rectOut->right	= (int) ((float)rectIn.right	* coef.x);
	rectOut->bottom	= (int) ((float)rectIn.bottom	* coef.y);

	return rectOut;
}

/**
 * Applique un d�calage � un rectangle.
 * @param _relativePosition La position relative de la fen�tre sur le moniteur
 * @param _offsetX Le d�calage � appliquer sur l'axe des abscisses
 * @param _offsetY Le d�calage � appliquer sur l'axe des ordonn�es
 */
Rect* MonUtils::ApplyOffest(Rect _relativePos, int _offsetX, int _offsetY)
{
	Rect *absPos = new Rect();
	absPos->left	= _relativePos.left	+ _offsetX;
	absPos->top		= _relativePos.top	+ _offsetY;
	absPos->right	= _relativePos.right	+ _offsetX;
	absPos->bottom	= _relativePos.bottom	+ _offsetY;
	
	return absPos;
}

/**
 * D�place la fen�tre vers l'�cran sp�cifi�
 * @param _window HWND La fen�tre � d�placer.
 * @param _monitor HMONITOR L'�cran source.
 * @param _monitor HMONITOR L'�cran vers lequel on veut d�placer la fen�tre.
 * @return true si le d�placement est effectu�.
 */
bool MonUtils::MoveWindow(HWND _window, HMONITOR _srcMonitor, HMONITOR _destMonitor)
{
	if (_srcMonitor == _destMonitor) {
		return false;
	}

	// R�cup�ration de la position du moniteur source
	Rect *srcMonPos = GetMonitorRect(_srcMonitor);
	if (srcMonPos) {
		srcMonPos->Display("srcMon");
	} else {
		std::cerr << "!!! GetMonitorRect(_srcMonitor) failed !!!" << std::endl;
	}
	
	// R�cup�ration de la position du nouveau moniteur
	Rect *destMonPos = GetMonitorRect(_destMonitor);
	if (destMonPos) {
		destMonPos->Display("destMon");
	} else {
		std::cerr << "!!! GetMonitorRect(_destMonitor) failed !!!" << std::endl;
	}

	// Calcul de la position relative de la fen�tre par rapport au moniteur source
	Rect *windowRelPos	= GetRelativePos(_window, *srcMonPos); 
	if (windowRelPos) {
		windowRelPos->Display("windowRelPos");
	} else {
		std::cerr << "!!! GetRelativePos(_window, *rectSrcMon) failed !!!" << std::endl;
	}

	// Calcul du coefficient d'agrandissement pour passer d'un moniteur � l'autre
	PointF coef	= GetRectCoef(*srcMonPos, *destMonPos); 
	coef.Display("coef");

	// Application du coef � la position relative de la fen�tre
	Rect *newRelPos = ApplyCoef(*windowRelPos, coef);
	if (newRelPos) {
		newRelPos->Display("newRelPos");
	} else {
		std::cout << "!!! ApplyCoef(*windowRelPos, coef) failed !!!" << std::endl;
	}

	// Calcul de la position absolue finale de la fen�tre
	Rect *newPos = ApplyOffest(*newRelPos, destMonPos->left, destMonPos->top); 
	if (newPos) {
		newPos->Display("newPos (final)");
	} else {
		std::cout << "!!! ApplyOffest(*newRelPos, rectDestMon->left, rectDestMon->top) failed !!!" << std::endl;
	}

	SetWindowPos(_window, nullptr, newPos->left, newPos->top, newPos->GetWidth(), newPos->GetHeight(), SWP_SHOWWINDOW);
	
	// Suppression des pointeurs cr��s
	delete(srcMonPos);
	delete(destMonPos);
	delete(windowRelPos);
	delete(newRelPos);
	delete(newPos);

	return true;
}

/**
 * D�place la fen�tre courante de l'�cran dans lequel elle se trouve vers l'�cran suivant.
 * @return true si le d�placement est effectu�.
 */
bool MonUtils::SwitchWindow()
{
	// On r�cup�re le nombre de moniteur
	int screenCount = GetSystemMetrics(SM_CMONITORS);
	std::cout << "Using " << screenCount << " screens." << std::endl << std::endl;

	if (screenCount < 2) { // Si on a pas au moins 2 moniteurs, on abandonne l'action en cours
		return false;
	}

	// On �tablit la liste des moniteurs
	EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, 0);  

	// R�cup�ration de la fen�tre au premier plan
	HWND foregroundWindow = GetForegroundWindow();
	// R�cup�ration de l'�cran courant
	HMONITOR currentMonitor = MonitorFromWindow(foregroundWindow, MONITOR_DEFAULTTONEAREST);
	// R�cup�ration du moniteur suivant
	HMONITOR nextMonitor = GetNextMonitor(currentMonitor);

	// D�placement de la fen�tre
	return MoveWindow(foregroundWindow, currentMonitor, nextMonitor);
}