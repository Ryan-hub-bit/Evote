/* Database implementations file */
#include "Database.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

_id_t storeElection(sqlite3 *db, Date deadline) {
   _id_t id = 0;
   sqlite3_stmt *stmt;
   const char *sql = "INSERT INTO Election(deadline_day,deadline_mon,\
                      deadline_year,status) VALUES (?, ?, ?, ?)";
   sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
   sqlite3_bind_int(stmt, 1, deadline.day);
   sqlite3_bind_int(stmt, 2, deadline.month);
   sqlite3_bind_int(stmt, 3, deadline.year);
   sqlite3_bind_int(stmt, 4, INACTIVE);
   sqlite3_step(stmt);
   if (sqlite3_finalize(stmt) == SQLITE_OK) {
      id = (_id_t)sqlite3_last_insert_rowid(db);
   }
   return id;
}

_id_t storeOffice(sqlite3 *db, _id_t election, char *name) {
   _id_t id = 0;
   sqlite3_stmt *stmt;
   const char *sql = "INSERT INTO Office(name, election) VALUES (?, ?)";
   sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
   sqlite3_bind_text(stmt, 1, name, (int)strnlen(name, MAX_NAME_LEN),
                     SQLITE_STATIC);
   sqlite3_bind_int(stmt, 2, election);
   sqlite3_step(stmt);
   if (sqlite3_finalize(stmt) == SQLITE_OK) {
      id = (_id_t)sqlite3_last_insert_rowid(db);
   }
   return id;
}

_id_t storeCandidate(sqlite3 *db, _id_t office, char *name, char *desc) {
   _id_t id = 0;
   sqlite3_stmt *stmt;
   const char *sql = "INSERT INTO Candidate(name,votes,office,info)\
                      VALUES (?, ?, ?, ?)";
   sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
   sqlite3_bind_text(stmt, 1, name, (int)strnlen(name, MAX_NAME_LEN),
                     SQLITE_STATIC);
   sqlite3_bind_int(stmt, 2, 0);
   sqlite3_bind_int(stmt, 3, office);
   sqlite3_bind_text(stmt, 4, desc, (int)strnlen(desc, MAX_DESC_LEN),
                     SQLITE_STATIC);
   sqlite3_step(stmt);
   if (sqlite3_finalize(stmt) == SQLITE_OK) {
      id = (_id_t)sqlite3_last_insert_rowid(db);
   }
   return id;
}

void addZip(sqlite3 *db, _id_t office, int zip) {
   sqlite3_stmt *stmt;
   const char *sql = "INSERT INTO AllowedZip(zip,office) VALUES (?, ?)";
   sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
   sqlite3_bind_int(stmt, 1, zip);
   sqlite3_bind_int(stmt, 2, office);
   sqlite3_step(stmt);
   sqlite3_finalize(stmt);
}

bool checkZip(sqlite3 *db, _id_t office, int zip) {
   int count;
   sqlite3_stmt *stmt;
   const char *sql = "SELECT COUNT(*) FROM AllowedZip WHERE\
                      zip=? AND office=?";
   sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
   sqlite3_bind_int(stmt, 1, zip);
   sqlite3_bind_int(stmt, 2, office);
   sqlite3_step(stmt);
   count = sqlite3_column_int(stmt, 0);
   sqlite3_finalize(stmt);
   return count > 0;
}

_id_t storeVoter(sqlite3 *db, char*name, char*county, int zip, Date dob) {
   _id_t id = 0;
   sqlite3_stmt *stmt;
   const char *sql = "INSERT INTO Registration(name,county,zip,\
                      dob_day,dob_mon,dob_year) VALUES (?, ?, ?, ?, ?, ?)";
   sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
   sqlite3_bind_text(stmt, 1, name, (int)strnlen(name, MAX_NAME_LEN),
                     SQLITE_STATIC);
   sqlite3_bind_text(stmt, 2, county, (int)strnlen(county, MAX_NAME_LEN),
                     SQLITE_STATIC);
   sqlite3_bind_int(stmt, 3, zip);
   sqlite3_bind_int(stmt, 4, dob.day);
   sqlite3_bind_int(stmt, 5, dob.month);
   sqlite3_bind_int(stmt, 6, dob.year);
   sqlite3_step(stmt);
   if (sqlite3_finalize(stmt) == SQLITE_OK) {
      id = (_id_t)sqlite3_last_insert_rowid(db);
   }
   return id;
}

void storeStatus(sqlite3 *db, _id_t election, Status new_status) {
   sqlite3_stmt *stmt;
   const char *sql = "UPDATE Election SET status=? WHERE id=?";
   sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
   sqlite3_bind_int(stmt, 1, (int)new_status);
   sqlite3_bind_int(stmt, 2, election);
   sqlite3_step(stmt);
   sqlite3_finalize(stmt);
   if (new_status == PUBLISHED) {
      const char *totals = "UPDATE Candidate SET votes=(\
                              SELECT COUNT(*) FROM Vote WHERE\
                              Vote.candidate=Candidate.id AND\
                              Vote.office=Candidate.office)";
      sqlite3_prepare_v2(db, totals, -1, &stmt, NULL);
      sqlite3_step(stmt);
      sqlite3_finalize(stmt);
   }
}

void deleteElection(sqlite3 *db, _id_t election) {
   sqlite3_stmt *stmt;
   const char *sql = "DELETE FROM Election WHERE id=?";
   sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
   sqlite3_bind_int(stmt, 1, election);
   sqlite3_step(stmt);
   sqlite3_finalize(stmt);
}

void getVoter(sqlite3 *db, _id_t voter_id, Registration* dest) {
   sqlite3_stmt *stmt;
   const char *sql = "SELECT name,county,zip,dob_day,dob_mon,dob_year\
                      FROM Registration WHERE id=?";
   sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
   sqlite3_bind_int(stmt, 1, voter_id);
   sqlite3_step(stmt);
   strncpy(dest->name, (char *)sqlite3_column_text(stmt, 0), MAX_NAME_LEN-1);
   strncpy(dest->county, (char *)sqlite3_column_text(stmt, 1),MAX_NAME_LEN-1);
   (dest->name)[MAX_NAME_LEN-1] = '\0';
   (dest->county)[MAX_NAME_LEN-1] = '\0';
   dest->zip = sqlite3_column_int(stmt, 2);
   (dest->dob).day = sqlite3_column_int(stmt, 3);
   (dest->dob).month = sqlite3_column_int(stmt, 4);
   (dest->dob).year = sqlite3_column_int(stmt, 5);
   sqlite3_finalize(stmt);
}

void getElection(sqlite3 *db, _id_t election_id, Election* dest) {
   sqlite3_stmt *stmt;
   const char *sql = "SELECT deadline_day,deadline_mon,deadline_year,status\
                      FROM Election WHERE id=?";
   sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
   sqlite3_bind_int(stmt, 1, election_id);
   sqlite3_step(stmt);
   (dest->deadline).day = sqlite3_column_int(stmt, 0);
   (dest->deadline).month = sqlite3_column_int(stmt, 1);
   (dest->deadline).year = sqlite3_column_int(stmt, 2);
   dest->status = (Status)sqlite3_column_int(stmt, 3);
   sqlite3_finalize(stmt);
}

void storeVote(sqlite3 *db, _id_t voter, _id_t candidate, _id_t office, char *reason) {
   sqlite3_stmt *stmt;
   const char *sql = "INSERT INTO Vote(voter,candidate,office,reason)\
                      VALUES (?, ?, ?, ?)";
   sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
   sqlite3_bind_int(stmt, 1, voter);
   sqlite3_bind_int(stmt, 2, candidate);
   sqlite3_bind_int(stmt, 3, office);
   sqlite3_bind_text(stmt, 4, reason, (int)strnlen(reason, MAX_NAME_LEN),
                     SQLITE_STATIC);
   sqlite3_step(stmt);
   sqlite3_finalize(stmt);
}

void updateVote(sqlite3 *db, _id_t voter, _id_t candidate, _id_t office) {
   sqlite3_stmt *stmt;
   const char *sql = "UPDATE Vote SET candidate=? WHERE voter=? AND office=?";
   sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
   sqlite3_bind_int(stmt, 2, voter);
   sqlite3_bind_int(stmt, 1, candidate);
   sqlite3_bind_int(stmt, 3, office);
   sqlite3_step(stmt);
   sqlite3_finalize(stmt);
}

int getVote(sqlite3 *db, _id_t voter_id, _id_t office_id) {
   int count;
   sqlite3_stmt *stmt;
   const char *sql = "SELECT COUNT(*) FROM Vote WHERE\
                      voter=? AND office=?";
   sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
   sqlite3_bind_int(stmt, 1, voter_id);
   sqlite3_bind_int(stmt, 2, office_id);
   sqlite3_step(stmt);
   count = sqlite3_column_int(stmt, 0);
   sqlite3_finalize(stmt);
   return count;
}

void getVoters(sqlite3 *db) {
   sqlite3_stmt *stmt;
   const char *sql = "SELECT name,county,zip,dob_day,dob_mon,dob_year\
                      FROM Registration";
   sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
   printf("[\n");
   bool is_first = false;
   while (sqlite3_step(stmt) != SQLITE_DONE) {
      if (!is_first) {
         is_first = true;
      } else {
         printf(",\n");
      }
      printf("{\"name\": \"%s\", \"county\": \"%s\", \"zip\": \"%d\", ",
             sqlite3_column_text(stmt, 0),
             sqlite3_column_text(stmt, 1),
             sqlite3_column_int(stmt, 2));
      printf("\"dob\": \"%d-%02d-%02d\"}",
             sqlite3_column_int(stmt, 5)+1900,
             sqlite3_column_int(stmt, 4),
             sqlite3_column_int(stmt, 3));
   }
   printf("\n]\n");
   sqlite3_finalize(stmt);
}

void storVote(sqlite3 *db, _id_t voter, _id_t candidate, _id_t office, char *ch) {
   char *err_msg = 0;
   char sqlStr[256];
   sprintf(sqlStr,"INSERT INTO Vote(reason,voter,candidate,office) VALUES('%s',%d, %d, %d)",ch,voter,candidate,office);
   int rc = sqlite3_exec(db, sqlStr, 0, 0, &err_msg);
   if(rc != SQLITE_OK) {
       fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
   }
}

void getElections(sqlite3 *db) {
   system("./database_helper.py"); /* U+1F914 */
}
