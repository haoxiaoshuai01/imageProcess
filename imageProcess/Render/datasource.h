#ifndef DATASOURCE_H
#define DATASOURCE_H
#include <QString>
#include <vector>
#include <map>
namespace  Engine{
class DataSource
{
public:
    DataSource(QString dir,QString coverName);
    ~DataSource();
    inline unsigned char *getCurrentData(){return imageData;}
    inline int getWidth(){return width;}
    inline int getHeight(){return height;}
    inline int getComp(){return comp;}
    void updateFrameToNext();
    inline size_t getCount(){return datas.size();}
    bool isLastFrame(){return currentID == (datas.size()-1);}
private:
    int width;
    int height;
    int comp;
    std::vector<QString> datas;
    size_t currentID;
    unsigned char * imageData;
    std::map<QString,unsigned char *> imageDataMap;

};
}
#endif // DATASOURCE_H
