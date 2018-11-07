/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2018 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "loadimage.h"

#include <QBuffer>
#include <QDebug>
#include <QDomElement>
#include <QFileInfo>
#include <QImage>
#include <QImageWriter>

#include "metasql.h"
#include "quuencode.h"

#define DEBUG false

LoadImage::LoadImage(const QString &name, const int order,
                     const bool system, const bool /*enabled*/,
                     const QString &comment, const QString &filename)
  : Loadable("loadimage", name, order, system, comment, filename)
{
  _pkgitemtype = "I";
  _stripBOM    = false;
}

LoadImage::LoadImage(const QDomElement &elem, const bool system,
                     QStringList &msg, QList<bool> &fatal)
  : Loadable(elem, system, msg, fatal)
{
  _pkgitemtype = "I";
  _stripBOM    = false;

  if (_name.isEmpty())
  {
    msg.append(TR("The image in %1 does not have a name.")
                       .arg(_filename));
    fatal.append(true);
  }

  if (elem.nodeName() != "loadimage")
  {
    msg.append(TR("Creating a LoadImage element from a %1 node.")
              .arg(elem.nodeName()));
    fatal.append(false);
  }

  if (elem.hasAttribute("grade") || elem.hasAttribute("order"))
  {
    msg.append(TR("Node %1 '%2' has a 'grade' or 'order' attribute "
                           "but these are ignored for images.")
                           .arg(elem.nodeName()).arg(elem.attribute("name")));
    fatal.append(false);
  }

  if (elem.hasAttribute("enabled"))
  {
    msg.append(TR("Node %1 '%2' has an 'enabled' "
                           "attribute which is ignored for images.")
                       .arg(elem.nodeName()).arg(elem.attribute("name")));
    fatal.append(false);
  }

}

int LoadImage::writeToDB(QByteArray &pData, const QString pPkgname, QString &errMsg)
{
  if (pData.isEmpty())
  {
    errMsg = TR("<font color=orange>The image %1 is empty.</font>") .arg(_name);
    return -2;
  }

  QByteArray encodeddata;
  if (DEBUG)
    qDebug() << "LoadImage::writeToDB(): image starts with" << pData.left(10);
  if (QString(pData.left(pData.indexOf("\n"))).contains(QRegExp("^\\s*begin \\d+ \\S+")))
  {
    if (DEBUG) qDebug("LoadImage::writeToDB() image is already uuencoded");
    encodeddata = pData;
  }
  else
  {
    QBuffer      imageBuffer;
    QImageWriter imageIo(&imageBuffer, QFileInfo(_filename).suffix().toLocal8Bit());

    if (DEBUG)
      qDebug() << "LoadImage::writeToDB() image has format" << imageIo.format();

    QImage image;
    image.loadFromData(pData);
    if (!imageIo.write(image))
    {
      errMsg = TR("<font color=orange>Error processing image %1:<br/>%2</font>")
                .arg(_name).arg(imageIo.errorString());
      return -3;
    }

    imageBuffer.close();
    encodeddata = QUUEncode(imageBuffer).toLatin1();
    if (DEBUG)
      qDebug() << "LoadImage::writeToDB() uuencoded image" << encodeddata.left(160);
  }

  _selectMql = new MetaSQLQuery("SELECT image_id, -1, -1"
                      "  FROM <? literal('tablename') ?> "
                      " WHERE (image_name=<? value('name') ?>);");

  _updateMql = new MetaSQLQuery("UPDATE <? literal('tablename') ?> "
                      "   SET image_data=<? value('source') ?>,"
                      "       image_descrip=<? value('notes') ?>"
                      " WHERE (image_id=<? value('id') ?>)"
                      " RETURNING image_id AS id;");

  _insertMql = new MetaSQLQuery("INSERT INTO <? literal('tablename') ?> ("
                      "   image_id, image_name, image_data, image_descrip"
                      ") VALUES ("
                      "  DEFAULT, <? value('name') ?>,"
                      "  <? value('source') ?>,"
                      "  <? value('notes') ?>"
                      ") RETURNING image_id AS id;");

  ParameterList params;
  params.append("tablename", "image");

  return Loadable::writeToDB(encodeddata, pPkgname, errMsg, params);
}
