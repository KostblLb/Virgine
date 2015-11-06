//#include "Level.h"
//#include <fstream>
//
//Level::Level(SDL_Rect* r, SDL_Renderer** ren)
//{
//	pMainRenderer = ren;
//	MainWindowRect = r;
//	GameObjects.resize(0);
//	
//}
//
//
//Level::~Level()
//{
//for each (GameObject* go in GameObjects){
//		delete go;
//	}
//}
//
///*не используется*/
//int Level::Load(int levnum){
//	std::fstream fs;
//	fs.open("levels.xml", std::fstream::in);
//	return 0;
//}
//
///*todo: скан системы на число файлов
//хэширование*/
//
///*int Level::Load(){
//	GameObjects.push_back(new GameObject("resources/background.png", "background"));
//	GameObjects.push_back(new GameObject("resources/apple.png", "apple"));
//	return 0;
//}*/
//int Level::Load(){
//	GameObjects.push_back(new GameObject("resources/background.png",MainRenderer, "background"));
//	GameObjects.push_back(new GameObject("resources/apple.png",MainRenderer, "apple"));
//	GameObjects.push_back(new GameObject("resources/common/player.png",MainRenderer, "player"));
//	return 0;
//}
//
///*HASH!!!!*/
//GameObject* Level::findWithTag(std::string t){
//	int i = 0;
//	while (GameObjects[i]->getTag() != t){
//		i++;
//		if (i >= GameObjects.size())
//			return NULL;
//	}
//	return GameObjects[i];
//}
//
//void Level::Update(){
//	/*std::pair<int, int> p;
//	while (!m_CollisionsQueue.empty()){
//		p = m_CollisionsQueue.front();
//		GameObjects[p.first]->doCollide(GameObjects[p.second]);
//		GameObjects[p.second]->doCollide(GameObjects[p.first]);
//
//		m_CollisionsQueue.pop();
//	}*/
//	for each (GameObject* obj in GameObjects)
//	{
//		obj->Update();
//	}
//}
