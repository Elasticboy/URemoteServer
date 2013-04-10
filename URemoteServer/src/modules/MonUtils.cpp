#include "MonUtils.h"

#include <iostream>

const auto ERROR_INDEX(-1);
const auto FIRST_MON_ID(0);

# if defined(WINDOWS_PLATFORM)
// TODO: comments FR => EN
// Initialisation du vecteur
std::vector<HMONITOR> MonUtils::monitors_ = std::vector<HMONITOR>();
#endif // defined

/**
 * D�place la fen�tre courante de l'�cran dans lequel elle se trouve vers l'�cran suivant.
 * @return true si le d�placement est effectu�.
 */
bool MonUtils::SwitchWindow()
{
# if defined(WINDOWS_PLATFORM)
	// On r�cup�re le nombre de moniteur
	const auto screenCount = GetSystemMetrics(SM_CMONITORS);
	std::cout << "Using " << screenCount << " screens." << std::endl << std::endl;

	if (screenCount < 2) { // Si on a pas au moins 2 moniteurs, on abandonne l'action en cours
		return false;
	}

	// On �tablit la liste des moniteurs
	EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, 0);

	// R�cup�ration de la fen�tre au premier plan
	auto foregroundWindow = GetForegroundWindow();
	// R�cup�ration de l'�cran courant
	auto currentMonitor = MonitorFromWindow(foregroundWindow, MONITOR_DEFAULTTONEAREST);
	// R�cup�ration du moniteur suivant
	auto nextMonitor = GetNextMonitor(currentMonitor);

	// D�placement de la fen�tre
	return MoveWindow(foregroundWindow, currentMonitor, nextMonitor);
# else
    // TODO: Implement MonUtils::SwitchWindow() on linux
    return false;
# endif
}


# if defined(WINDOWS_PLATFORM)
/**
 * CALLBACK permettant de lister les moniteurs
 */
BOOL CALLBACK MonUtils::MonitorEnumProc(HMONITOR monitor, HDC hdcMonitor, LPRECT lpMonitorRect, LPARAM dwData)
{
	monitors_.push_back(monitor);
	return TRUE;
}

/**
 * @param monitor Le moniteur dont on veut connaitre l'index
 * @return L'index du moniteur courant.
 * Retourne -1 en cas d'erreur.
 */
int MonUtils::GetMonitorIndex(HMONITOR monitor)
{
	const int monitorsCount = monitors_.size();
	for (auto i = 0; i < monitorsCount; i++) {
		if (monitors_[i] == monitor)
			return i;
	}

	return ERROR_INDEX;
}

/**
 * @param monitor Le moniteur pr�c�dent celui dont on veut connaitre l'index.
 * @return L'index du moniteur suivant.
 * Si on est sur le dernier moniteur, on boucle sur le premier.
 * Retourne -1 en cas d'erreur.
 */
int MonUtils::GetNextMonitorIndex(HMONITOR monitor)
{
	const auto monitorsCount	= GetSystemMetrics(SM_CMONITORS);
	const auto currentIndex		= GetMonitorIndex(monitor);

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
 * @param monitor Le moniteur pr�c�dent celui que l'on veut r�cup�rer.
 * @return Le moniteur suivant celui pass� en param�tre.
 */
HMONITOR MonUtils::GetNextMonitor(HMONITOR monitor)
{
	const auto nextMonitorIndex = GetNextMonitorIndex(monitor);

	if (nextMonitorIndex == ERROR_INDEX) {
		return nullptr;
	}

	return monitors_[nextMonitorIndex];
}

/**
 * @param monitor Le moniteur dont on veut connaitre les dimensions.
 * @return Le rectangle d�limitant la position et les dimensions du moniteur.
 * Retourne nullptr en cas d'erreur.
 */
rect* MonUtils::GetMonitorRect(HMONITOR monitor)
{
	MONITORINFO monitorInfo;
	monitorInfo.cbSize = sizeof(monitorInfo);

	if (GetMonitorInfo(monitor, &monitorInfo) != 0) {
		return new rect(monitorInfo.rcMonitor);
	}

	return nullptr;
}

/**
 * @param window La fen�tre dont on veut connaitre la position relative.
 * @param rectMon La position du moniteur.
 * @return La position relative de la fen�tre par rappot au moniteur.
 * Retourne nullptr en cas d'erreur.
 */
rect* MonUtils::GetRelativePos(HWND window, rect rectMon)
{
	// R�cuperation de la position de la fen�tre
	RECT windowPos;
	if ( GetWindowRect(window, &windowPos)  !=0 ) {
		auto* relativePos = new rect();
		// Calcul de l'offset en pixels
		relativePos->left	= windowPos.left	- rectMon.left;
		relativePos->top	= windowPos.top		- rectMon.top;
		relativePos->right	= windowPos.right	- rectMon.left;
		relativePos->bottom	= windowPos.bottom	- rectMon.top;
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
pointf MonUtils::GetRectCoef(rect rect1, rect rect2)
{
	pointf coef;
	coef.x = (float) (rect2.get_width())	/ (float) (rect1.get_width());
	coef.y = (float) (rect2.get_height())	/ (float) (rect1.get_height());

	return coef;
}

/**
 * Application d'un coefficient � un rectange.
 * @param rectIn Le rectangle � redimensionner.
 * @param coef Le coefficient � appliquer.
 * @return Le rectangle redimensionn�.
 */
rect* MonUtils::ApplyCoef(rect rectIn, pointf coef)
{
	auto* rectOut = new rect();
	rectOut->left	= (int) ((float)rectIn.left		* coef.x);
	rectOut->top	= (int) ((float)rectIn.top		* coef.y);
	rectOut->right	= (int) ((float)rectIn.right	* coef.x);
	rectOut->bottom	= (int) ((float)rectIn.bottom	* coef.y);

	return rectOut;
}

/**
 * Applique un d�calage � un rectangle.
 * @param relativePosition La position relative de la fen�tre sur le moniteur
 * @param offsetX Le d�calage � appliquer sur l'axe des abscisses
 * @param offsetY Le d�calage � appliquer sur l'axe des ordonn�es
 */
rect* MonUtils::ApplyOffest(rect relativePos, const int& offsetX, const int& offsetY)
{
	auto* absPos = new rect();
	absPos->left	= relativePos.left		+ offsetX;
	absPos->top		= relativePos.top		+ offsetY;
	absPos->right	= relativePos.right		+ offsetX;
	absPos->bottom	= relativePos.bottom	+ offsetY;

	return absPos;
}

/**
 * D�place la fen�tre vers l'�cran sp�cifi�
 * @param window HWND La fen�tre � d�placer.
 * @param monitor HMONITOR L'�cran source.
 * @param monitor HMONITOR L'�cran vers lequel on veut d�placer la fen�tre.
 * @return true si le d�placement est effectu�.
 */
bool MonUtils::MoveWindow(HWND window, HMONITOR srcMonitor, HMONITOR destMonitor)
{
	if (srcMonitor == destMonitor) {
		return false;
	}

	// R�cup�ration de la position du moniteur source
	auto* srcMonPos = GetMonitorRect(srcMonitor);
	if (srcMonPos) {
		srcMonPos->display("srcMon");
	} else {
		std::cerr << "!!! GetMonitorRect(_srcMonitor) failed !!!" << std::endl;
	}

	// R�cup�ration de la position du nouveau moniteur
	auto* destMonPos = GetMonitorRect(destMonitor);
	if (destMonPos) {
		destMonPos->display("destMon");
	} else {
		std::cerr << "!!! GetMonitorRect(_destMonitor) failed !!!" << std::endl;
	}

	// Calcul de la position relative de la fen�tre par rapport au moniteur source
	auto* windowRelPos	= GetRelativePos(window, *srcMonPos);
	if (windowRelPos) {
		windowRelPos->display("windowRelPos");
	} else {
		std::cerr << "!!! GetRelativePos(_window, *rectSrcMon) failed !!!" << std::endl;
	}

	// Calcul du coefficient d'agrandissement pour passer d'un moniteur � l'autre
	auto coef	= GetRectCoef(*srcMonPos, *destMonPos);
	coef.display("coef");

	// Application du coef � la position relative de la fen�tre
	auto* newRelPos = ApplyCoef(*windowRelPos, coef);
	if (newRelPos) {
		newRelPos->display("newRelPos");
	} else {
		std::cout << "!!! ApplyCoef(*windowRelPos, coef) failed !!!" << std::endl;
	}

	// Calcul de la position absolue finale de la fen�tre
	auto* newPos = ApplyOffest(*newRelPos, destMonPos->left, destMonPos->top);
	if (newPos) {
		newPos->display("newPos (final)");
	} else {
		std::cout << "!!! ApplyOffest(*newRelPos, rectDestMon->left, rectDestMon->top) failed !!!" << std::endl;
	}

	SetWindowPos(window, nullptr, newPos->left, newPos->top, newPos->get_width(), newPos->get_height(), SWP_SHOWWINDOW);

	// Suppression des pointeurs cr��s
	delete(srcMonPos);
	delete(destMonPos);
	delete(windowRelPos);
	delete(newRelPos);
	delete(newPos);

	return true;
}
#endif
