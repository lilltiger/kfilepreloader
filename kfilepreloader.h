//
// C++ Interface: filepreload
//
// Description: An filepreloader
//
//
// Author: Peter Bengtsson <dev@lilltiger.se>, (C) 2008
//
// Copyright: GPLv3
//

#include <QDir>
#include <QtConcurrentRun>
#include <QObject>
#include <QString>
#include <QLinkedList>

#include "qcirculariterator.h"

#ifndef KFILEPRELOAD_H
#define KFILEPRELOAD_H

//TODO: Implement randomize, clean up some unnessesary checks that was added while tracking down a bug.


class KFilePreload : public QObject
{
	Q_OBJECT

	public:
		KFilePreload();

		~KFilePreload();

		void setFilters(QStringList filters);
		void next();

		void previous();

		void addDirectory(QString path, bool recrusive);

		void clear();

		void randomize();

		void setPreloadAmouth(int preload);
	signals:
		void loadItem(QString path, bool to_front = false);
		void removeItem(QString path, bool from_front = true);

	private: // Functions

		void initIterators();

		void addRecursiveDir(const QString &path);

		void addDir(QString path);


		void preloadForward();

		void preloadBackward();

		// Remove the preloaded items from the front
		void removePreload(bool from_front);

		void futureCleanUp();


	private:
		//NOTE: We must use QLinkedList<QString> and not QStringList cos we rely upon iterators not invalidating upon insert
		CircularIterator< QLinkedList<QString> > files_ite_next_;
		CircularIterator< QLinkedList<QString> > files_ite_previous_;

		QStringList filters_;
		QLinkedList<QString> files_;

		int preload_;
		int steps_left_preload_;
		int num_preloaded_;

		bool exit_;
		QLinkedList< QFuture<void> > futures_;
};

#endif
