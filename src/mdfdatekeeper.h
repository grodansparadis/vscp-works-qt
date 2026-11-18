// mdfdatekeeper.h
//
// A date in an MDF is free text (ISO 8601 recommended). A QDateEdit can only
// show a valid date, and shows some default date when the text can not be
// read as one. To never change a date the user did not touch, the text from
// the MDF is kept and written back unless the date in the editor was changed.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2026 Ake Hedman, the VSCP project <info@vscp.org>
//

#ifndef MDFDATEKEEPER_H
#define MDFDATEKEEPER_H

#include <QDate>
#include <QDateEdit>
#include <QString>

#include <string>

class MdfDateKeeper {
public:
  /// Show the date from the MDF in the editor and remember it
  void load(QDateEdit* edit, const std::string& date)
  {
    m_original  = date;
    QString str = QString::fromStdString(date).trimmed();
    QDate d     = QDate::fromString(str, Qt::ISODate);
    if (!d.isValid()) {
      d = QDate::fromString(str, "yy-MM-dd");
    }
    if (d.isValid()) {
      edit->setDate(d);
    }
    m_shown = edit->date();
  }

  /// The date to store: the original text if the editor was not changed
  std::string value(const QDateEdit* edit) const
  {
    if (edit->date() == m_shown) {
      return m_original;
    }
    return edit->date().toString(Qt::ISODate).toStdString();
  }

private:
  std::string m_original;
  QDate m_shown;
};

#endif
