#include <assert.h> /*asserts*/
#include <stdlib.h>	/*size_t*/
#include <unistd.h> /* getpid */
#include "uid.h"


const nuid_t g_bad_uid = {-1, {0}, 0};


nuid_t UidCreate()
{
	nuid_t new = {0};
	int check = 0;

	static size_t counter = 0;

	new.pid = getpid();
	check = gettimeofday(&new.time, NULL);
	assert(!check);
	(void)check;

	++counter;
	new.counter = counter;

	return new;
}

int UidIsSame(nuid_t uid1, nuid_t uid2)
{
	return (uid1.pid == uid2.pid && uid1.time.tv_sec == uid2.time.tv_sec && 
			uid1.time.tv_usec == uid2.time.tv_usec &&
			uid1.counter == uid2.counter);
}

int UidIsValid(nuid_t checked_uid)
{
	return (!(checked_uid.pid 		   == g_bad_uid.pid) &&
			!(checked_uid.time.tv_sec  == g_bad_uid.time.tv_sec) &&
			!(checked_uid.time.tv_usec == g_bad_uid.time.tv_usec) && 
			!(checked_uid.counter      == g_bad_uid.counter));
}
