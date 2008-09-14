 
#include "kfilepreloader.h"

KFilePreload::KFilePreload()
	: files_ite_next_(files_)
	, files_ite_previous_(files_)
	, preload_(3)
	, steps_left_preload_(3)
	, num_preloaded_(0)
	, exit_(false)
{

}

KFilePreload::~KFilePreload()
{
	// wait for all qfutures then exit gracefully..
	exit_ = true;

	QLinkedList< QFuture<void> >::iterator it( futures_.begin() );
	
	for(; it != futures_.end(); ++it)
	{
		it->waitForFinished();
	}
}

void KFilePreload::setFilters(QStringList filters)
{
	filters_ = filters;
}

void KFilePreload::next()
{
	if( steps_left_preload_ == preload_*2 )
	{
		steps_left_preload_ = preload_;
		futures_.push_back( QtConcurrent::run(this,&KFilePreload::preloadForward) );
		futures_.push_back( QtConcurrent::run(this,&KFilePreload::removePreload,true) );
	} else {
		++steps_left_preload_;
	}
}

void KFilePreload::previous()
{
	if( steps_left_preload_ == 0 )
	{
		steps_left_preload_ = preload_;
		futures_.push_back( QtConcurrent::run(this,&KFilePreload::preloadBackward) );
		futures_.push_back( QtConcurrent::run(this,&KFilePreload::removePreload,false) );
	} else {
		--steps_left_preload_;
	}
}

void KFilePreload::addDirectory(QString path, bool recrusive = false)
{
	if(exit_) return;
	if(recrusive)
	{
		futures_.push_back( QtConcurrent::run(this,&KFilePreload::addRecursiveDir,path) );
	} else {
		futures_.push_back( QtConcurrent::run(this,&KFilePreload::addDir,path) );
	}
}

void KFilePreload::clear()
{
	files_.clear();
}

void KFilePreload::randomize()
{
// 			lock_.lockForWrite();
// 			KRandomSequence randomSequence;
// 			randomSequence.randomize(files_);
// 			lock_.unlock();
}

void KFilePreload::setPreloadAmouth(int preload)
{
	steps_left_preload_ = preload;
	preload_ = preload;
}

void KFilePreload::initIterators()
{
	switch( files_.count() )
	{
		case 1:
			files_ite_next_.toFront();
			files_ite_previous_.toFront();
			emit loadItem( QString(*files_ite_next_) );
			++num_preloaded_;
			break;
		case 2:
			files_ite_next_.toBack();
			files_ite_previous_.toFront();
			emit loadItem( QString(*files_ite_next_) );
			++num_preloaded_;
			break;
		case 3:
			files_ite_next_.toBack();
			files_ite_previous_.toFront();
			emit loadItem( QString(*files_ite_next_) );
			++num_preloaded_;
			break;
	}
}

void KFilePreload::addRecursiveDir(const QString &path)
{
	addDir(path);
	QDir dir(path);

	foreach (const QString &subDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
		if(exit_) break;
		addRecursiveDir(path + '/' + subDir);
	}
	futureCleanUp();
}

void KFilePreload::addDir(QString path)
{
	bool set_ite = false;
	if( files_.isEmpty() ) set_ite = true;

	//Iterator pointing to the last item before the insertion of the new items.
	QLinkedList<QString>::iterator ite_last( files_.end()-1 );

	QDir dir(path);

	dir.setNameFilters(filters_);
	if (dir.entryList().isEmpty())  {
		return;
	}

	foreach (const QString &imageFile, dir.entryList(QDir::Files)) {
		files_.append(QString(path + '/' + imageFile));
		if(exit_) return;
		initIterators();
	}

	if( num_preloaded_ > 2 && num_preloaded_ < preload_*2 )
	{
		futures_.push_back( QtConcurrent::run(this,&KFilePreload::preloadForward) );
		futures_.push_back( QtConcurrent::run(this,&KFilePreload::preloadBackward) );
	}

	futureCleanUp();
}


void KFilePreload::preloadForward()
{
	for(int i = 0; i < preload_+1 && ++files_ite_previous_ != files_ite_next_; ++i)
	{
		if(exit_) break;
		++num_preloaded_;
		emit loadItem( *files_ite_previous_ );
	}
	futureCleanUp();
}

void KFilePreload::preloadBackward()
{
	for(int i = 0; i < preload_+1 && --files_ite_previous_ != files_ite_next_; ++i)
	{
		if(exit_) break;
		++num_preloaded_;
		emit loadItem( *files_ite_previous_ );
	}
	futureCleanUp();
}

// Remove the preloaded items from the front
void KFilePreload::removePreload(bool from_front)
{
	// if the iterators collide we cant remove items, as they all should be preloaded
	for(int i = 0; i < preload_+1 && files_ite_previous_ != files_ite_next_; ++i)
	{
		if(exit_) break;
		emit removeItem( *(from_front ? files_ite_previous_++ :  files_ite_previous_-- ) ,true);
	}
	futureCleanUp();
}

void KFilePreload::futureCleanUp()
{
	QLinkedList< QFuture<void> >::iterator it( futures_.begin() );
	for(; it != futures_.end(); ++it)
	{
		if(!it->isRunning())
		{
			it = futures_.erase(it);
		}
	}
}

#include "kfilepreloader.moc"
