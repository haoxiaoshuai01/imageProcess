#include "datasource.h"
#include "stb/stb_image.h"
#include <QDir>
#include "qdebug.h"
#include "stdio.h"
#include <iostream>
namespace  Engine{
DataSource::DataSource(QString dir,QString coverName)
{
    //stbi_set_flip_vertically_on_load(true);
    QFileInfoList fileInfoList = QDir(dir).entryInfoList();
    for(auto& fileInfo : fileInfoList)
        {
            if(fileInfo.isFile())
            {
                if(fileInfo.fileName() != coverName)
                datas.push_back( fileInfo.absoluteFilePath());

            }
        }

    currentID = 0;
    //stbi_set_flip_vertically_on_load(true);
    if(imageDataMap.count( datas[currentID]) == 0 )
    {

        FILE *f;
        if(0!=fopen_s(&f,datas[currentID].toStdString().data(), "rb"))
        {
          std::cout<<"error datasource open failed"<<std::endl;
        }
        fseek(f,10,SEEK_SET);

        auto stbData = stbi_load_from_file(f,
                                        &width, &height, &comp, 0);
        fclose(f);
        imageDataMap[datas[currentID]] = stbData;
        imageData =stbData;

    }else
    {
        imageData =imageDataMap[datas[currentID]];
    }
}

DataSource::~DataSource()
{
   for(auto item:imageDataMap)
   {
     stbi_image_free(item.second);
   }
}

void DataSource::updateFrameToNext()
{
    if(datas.size() == 0)
        return;

    currentID++;
    currentID =currentID%datas.size();

    if(imageDataMap.count( datas[currentID]) == 0 )
    {
        FILE *f;
        if(0!=fopen_s(&f,datas[currentID].toStdString().data(), "rb"))
        {
          std::cout<<"error datasource open failed"<<std::endl;
        }
        fseek(f,10,SEEK_SET);

        auto stbData = stbi_load_from_file(f,
                                        &width, &height, &comp, 0);
        fclose(f);
        imageDataMap[datas[currentID]] = stbData;
        imageData =stbData;

    }else
    {
        imageData =imageDataMap[datas[currentID]];
    }





}
}
