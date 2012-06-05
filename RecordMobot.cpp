#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "RecordMobot.h"

CRecordMobot::CRecordMobot(char *name)
{
	_numMotions = 0;
	_motions = (struct motion_s**)malloc(sizeof(struct motion_s*) * 100);
	_numMotionsAllocated = 100;
	strcpy(_name, name);
}

void RecordMobot_init(recordMobot_t* mobot, char *name)
{
  Mobot_init((mobot_t*)mobot);
  mobot->numMotions = 0;
  mobot->motions = (struct motion_s**)malloc(sizeof(struct motion_s*) * 100); 
  mobot->numMotionsAllocated = 100;
  strcpy(mobot->name, name);
}

CRecordMobot::~CRecordMobot(void)
{
	free(_motions);
}

void RecordMobot_destroy(recordMobot_t* mobot)
{
  free(mobot->motions);
}

int CRecordMobot::connectWithAddress(const char address[], int channel)
{
  strcpy(_address, address);
  return CMobot::connectWithAddress(address, channel);
}

int RecordMobot_connectWithAddress(recordMobot_t* mobot, const char address[], int channel)
{
  strcpy(mobot->address, address);
  return Mobot_connectWithAddress((mobot_t*)mobot, address, channel);
}

const char* CRecordMobot::getAddress()
{
  return _address;
}

const char* RecordMobot_getAddress(recordMobot_t* mobot)
{
  return mobot->address;
}

int CRecordMobot::record(void)
{
	/* Get the robots positions */
	double angles[4];
	getJointAnglesAbs(angles[0], angles[1], angles[2], angles[3]);
	struct motion_s* motion;
	motion = (struct motion_s*)malloc(sizeof(struct motion_s));
	motion->motionType = MOTION_POS;
	motion->data.pos[0] = angles[0];
	motion->data.pos[1] = angles[1];
	motion->data.pos[2] = angles[2];
	motion->data.pos[3] = angles[3];
	motion->name = strdup("Pose");
	_motions[_numMotions] = motion;
	_numMotions++;
	
	return 0;
}

int RecordMobot_record(recordMobot_t* mobot)
{
	/* Get the robots positions */
	double angles[4];
	Mobot_getJointAnglesAbs((mobot_t*)mobot, &angles[0], &angles[1], &angles[2], &angles[3]);
	struct motion_s* motion;
	motion = (struct motion_s*)malloc(sizeof(struct motion_s));
	motion->motionType = MOTION_POS;
	motion->data.pos[0] = angles[0];
	motion->data.pos[1] = angles[1];
	motion->data.pos[2] = angles[2];
	motion->data.pos[3] = angles[3];
	motion->name = strdup("Pose");
	mobot->motions[mobot->numMotions] = motion;
	mobot->numMotions++;
	return 0;
}

int CRecordMobot::addDelay(double seconds)
{
	struct motion_s* motion;
	motion = (struct motion_s*)malloc(sizeof(struct motion_s));
	motion->motionType = MOTION_SLEEP;
	motion->data.sleepDuration = seconds;
	motion->name = strdup("Delay");
	_motions[_numMotions] = motion;
	_numMotions++;
	return 0;
}

int RecordMobot_addDelay(recordMobot_t* mobot, double seconds)
{
	struct motion_s* motion;
	motion = (struct motion_s*)malloc(sizeof(struct motion_s));
	motion->motionType = MOTION_SLEEP;
	motion->data.sleepDuration = seconds;
	motion->name = strdup("Delay");
	mobot->motions[mobot->numMotions] = motion;
	mobot->numMotions++;
	return 0;
}

int CRecordMobot::play(int index)
{
	if (index < 0 || index >= _numMotions) {
		return -1;
	}
	if(_motions[index]->motionType == MOTION_POS) {
		return moveToAbsNB(
			_motions[index]->data.pos[0],
			_motions[index]->data.pos[1],
			_motions[index]->data.pos[2],
			_motions[index]->data.pos[3]
		);
	} else if (_motions[index]->motionType == MOTION_SLEEP) {
		usleep(_motions[index]->data.sleepDuration * 1000000);
		return 0;
	}
}

int RecordMobot_play(recordMobot_t* mobot, int index)
{
	if (index < 0 || index >= mobot->numMotions) {
		return -1;
	}
	if(mobot->motions[index]->motionType == MOTION_POS) {
		return Mobot_moveToAbsNB( (mobot_t*)mobot,
			mobot->motions[index]->data.pos[0],
			mobot->motions[index]->data.pos[1],
			mobot->motions[index]->data.pos[2],
			mobot->motions[index]->data.pos[3]
		);
	} else if (mobot->motions[index]->motionType == MOTION_SLEEP) {
		usleep(mobot->motions[index]->data.sleepDuration * 1000000);
		return 0;
	}
}

int CRecordMobot::getMotionType(int index)
{
	if (index < 0 || index >= _numMotions) {
		return -1;
	}
	return _motions[index]->motionType;
}

int RecordMobot_getMotionType(recordMobot_t* mobot, int index)
{
	if (index < 0 || index >= mobot->numMotions) {
		return -1;
	}
	return mobot->motions[index]->motionType;
}

int CRecordMobot::getMotionString(int index, char* buf)
{
  switch(_motions[index]->motionType) {
    case MOTION_POS:
      sprintf(buf, "%s.moveToNB(%.2lf, %.2lf, %.2lf, %.2lf);",
          _name,
          _motions[index]->data.pos[0],
          _motions[index]->data.pos[1],
          _motions[index]->data.pos[2],
          _motions[index]->data.pos[3] );
      break;
    case MOTION_SLEEP:
      sprintf(buf, "msleep(%d);", (int)(_motions[index]->data.sleepDuration * 1000));
      break;
  }
	return 0;
}

int RecordMobot_getMotionString(recordMobot_t* mobot, int index, char* buf)
{
  switch(mobot->motions[index]->motionType) {
    case MOTION_POS:
      sprintf(buf, "%s.moveToNB(%.2lf, %.2lf, %.2lf, %.2lf);",
          mobot->name,
          mobot->motions[index]->data.pos[0],
          mobot->motions[index]->data.pos[1],
          mobot->motions[index]->data.pos[2],
          mobot->motions[index]->data.pos[3] );
      break;
    case MOTION_SLEEP:
      sprintf(buf, "msleep(%d);", (int)(mobot->motions[index]->data.sleepDuration * 1000));
      break;
  }
	return 0;
}

const char* CRecordMobot::getMotionName(int index)
{
	if(index < 0 || index >= _numMotions) {
		return NULL;
	}
	return _motions[index]->name;
}

const char* RecordMobot_getMotionName(recordMobot_t* mobot, int index)
{
	if(index < 0 || index >= mobot->numMotions) {
		return NULL;
	}
	return mobot->motions[index]->name;
}

int CRecordMobot::setMotionName(int index, const char* name)
{
	if(index < 0 || index >= _numMotions) {
		return -1;
	}
	free (_motions[index]->name);
	_motions[index]->name = strdup(name);
	return 0;
}

int RecordMobot_setMotionName(recordMobot_t* mobot, int index, const char* name)
{
	if(index < 0 || index >= mobot->numMotions) {
		return -1;
	}
	free (mobot->motions[index]->name);
	mobot->motions[index]->name = strdup(name);
	return 0;
}

int CRecordMobot::removeMotion(int index, bool releaseData)
{
	if(index < 0 || index >= numMotions()) {
		return -1;
	}
	/* Free the motion */
  if(releaseData) {
    free(_motions[index]);
  }
	/* Shift everything lower than the motion up by one */
	int i;
	for(i = index+1; i < _numMotions; i++) {
		_motions[i-1] = _motions[i];
	}
	_numMotions--;
	return 0;
}

int RecordMobot_removeMotion(recordMobot_t* mobot, int index, bool releaseData)
{
	if(index < 0 || index >= mobot->numMotions) {
		return -1;
	}
	/* Free the motion */
  if(releaseData) {
    free(mobot->motions[index]);
  }
	/* Shift everything lower than the motion up by one */
	int i;
	for(i = index+1; i < mobot->numMotions; i++) {
		mobot->motions[i-1] = mobot->motions[i];
	}
	mobot->numMotions--;
	return 0;
}

int CRecordMobot::clearAllMotions()
{
  int i;
  for(i = 0; i < _numMotions; i++) {
    free(_motions[i]);
  }
  _numMotions = 0;
  return 0;
}

int RecordMobot_clearAllMotions(recordMobot_t* mobot)
{
  int i;
  for(i = 0; i < mobot->numMotions; i++) {
    free(mobot->motions[i]);
  }
  mobot->numMotions = 0;
  return 0;
}

int CRecordMobot::moveMotion(int fromindex, int toindex)
{
  if(fromindex < 0 || fromindex >= _numMotions) {
    return -1;
  }
  if(toindex < 0 || toindex >= _numMotions) {
    return -1;
  }
  if(fromindex == toindex) {
    return 0;
  }
  /* Save the motion at fromindex */
  struct motion_s* motion = _motions[fromindex];
  /* Now, remove _motions[fromindex] */
  removeMotion(fromindex, false);
  //if(toindex > fromindex) {toindex--;}
  /* Make a space for the new motion at 'toindex' */
  int i;
  for(i = _numMotions - 1; i >= toindex; i--) {
    _motions[i+1] = _motions[i];
  }
  _motions[toindex] = motion;
  _numMotions++;
  
  return 0;
}

int RecordMobot_moveMotion(recordMobot_t* mobot, int fromindex, int toindex)
{
  if(fromindex < 0 || fromindex >= mobot->numMotions) {
    return -1;
  }
  if(toindex < 0 || toindex >= mobot->numMotions) {
    return -1;
  }
  if(fromindex == toindex) {
    return 0;
  }
  /* Save the motion at fromindex */
  struct motion_s* motion = mobot->motions[fromindex];
  /* Now, remove mobot->motions[fromindex] */
  RecordMobot_removeMotion(mobot, fromindex, false);
  //if(toindex > fromindex) {toindex--;}
  /* Make a space for the new motion at 'toindex' */
  int i;
  for(i = mobot->numMotions - 1; i >= toindex; i--) {
    mobot->motions[i+1] = mobot->motions[i];
  }
  mobot->motions[toindex] = motion;
  mobot->numMotions++;
  
  return 0;
}

int CRecordMobot::numMotions()
{
	return _numMotions;
}
