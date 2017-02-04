#include "friends.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


/*
 * Create a new user with the given name.  Insert it at the tail of the list 
 * of users whose head is pointed to by *user_ptr_add.
 *
 * Return:
 *   - 0 on success.
 *   - 1 if a user by this name already exists in this list.
 *   - 2 if the given name cannot fit in the 'name' array
 *       (don't forget about the null terminator).
 */
int create_user(const char *name, User **user_ptr_add) {
    if (strlen(name) >= MAX_NAME) {
        return 2;
    }

    User *new_user = malloc(sizeof(User));
    if (new_user == NULL) {
        perror("malloc");
        exit(1);
    }
    strncpy(new_user->name, name, MAX_NAME); // name has max length MAX_NAME - 1
	int i;
	/*
    for (i = 0; i < MAX_NAME; i++) {
        new_user->profile_pic[i] = '\0';
    }*/

    new_user->first_post = NULL;
    new_user->next = NULL;
    for (i = 0; i < MAX_FRIENDS; i++) {
        new_user->friends[i] = NULL;
    }

    // Add user to list
    User *prev = NULL;
    User *curr = *user_ptr_add;
    while (curr != NULL && strcmp(curr->name, name) != 0) {
        prev = curr;
        curr = curr->next;
    }

    if (prev == NULL) {
        *user_ptr_add = new_user;
        return 0;
    } else if (curr != NULL) {
        free(new_user);
        return 1;
    } else {
        prev->next = new_user;
        return 0;
    }
}


/* 
 * Return a pointer to the user with this name in
 * the list starting with head. Return NULL if no such user exists.
 *
 * NOTE: You'll likely need to cast a (const User *) to a (User *)
 * to satisfy the prototype without warnings.
 */
User *find_user(const char *name, const User *head) {
/*    const User *curr = head;
    while (curr != NULL && strcmp(name, curr->name) != 0) {
        curr = curr->next;
    }

    return (User *)curr;
*/
	const User *curr = head;
    while (curr != NULL && strcmp(name, curr->name) != 0) {
        curr = curr->next;
    }

    return (User *)curr;
}


/*
 * Print the usernames of all users in the list starting at curr.
 * Names should be printed to standard output, one per line.
 */
char* list_users(const User *curr) {
	int alloc_num = strlen("User List\n");
    const User* temp_curr = curr;
    while (temp_curr != NULL) {
		alloc_num += strlen(temp_curr->name) + 1;
		//alloc_num += strlen("\t\n");
        temp_curr = temp_curr->next;
    }
    char* buf = malloc(sizeof(char) * (alloc_num + 1));
    char * current = buf;
    strcpy(buf, "User List\n");
    current = &(current[strlen("User List\n")]);
    while (curr != NULL) {
		snprintf(current, alloc_num, "\t%s", curr->name);
		current = &(current[strlen(curr->name)+1]);
        curr = curr->next;
    }
    buf[alloc_num] = '\0';
    return buf;
}



/* 
 * Make two users friends with each other.  This is symmetric - a pointer to 
 * each user must be stored in the 'friends' array of the other.
 *
 * New friends must be added in the first empty spot in the 'friends' array.
 *
 * Return:
 *   - 0 on success.
 *   - 1 if the two users are already friends.
 *   - 2 if the users are not already friends, but at least one already has
 *     MAX_FRIENDS friends.
 *   - 3 if the same user is passed in twice.
 *   - 4 if at least one user does not exist.
 *
 * Do not modify either user if the result is a failure.
 * NOTE: If multiple errors apply, return the *largest* error code that applies.
 */
int make_friends(const char *name1, const char *name2, User *head) {
    User *user1 = find_user(name1, head);
    User *user2 = find_user(name2, head);

    if (user1 == NULL || user2 == NULL) {
        return 4;
    } else if (user1 == user2) { // Same user
        return 3;
    }

    int i, j;
    for (i = 0; i < MAX_FRIENDS; i++) {
        if (user1->friends[i] == NULL) { // Empty spot
            break;
        } else if (user1->friends[i] == user2) { // Already friends.
            return 1;
        }
    }

    for (j = 0; j < MAX_FRIENDS; j++) {
        if (user2->friends[j] == NULL) { // Empty spot
            break;
        } 
    }

    if (i == MAX_FRIENDS || j == MAX_FRIENDS) { // Too many friends.
        return 2;
    }
    user1->friends[i] = user2;
    user2->friends[j] = user1;
    return 0;
}




/*
 *  Print a post.
 *  Use localtime to print the time and date.
 */
char* print_post(const Post *post) {
    if (post == NULL) {
        return "Empty post";
    }
    int buf_size = 0;
    buf_size += strlen("From: ") + 1;
    // Print author
    // printf("From: %s\n", post->author);
    buf_size += strlen(post->author);
    // Print date
    //printf("Date: %s\n", asctime(localtime(post->date)));
	buf_size += strlen("Date: ") + 1;
	buf_size += strlen(asctime(localtime(post->date)));
    // Print message
    buf_size += strlen(post->contents)+1;
    // printf("%s\n", post->contents);
    char* buf = malloc(sizeof(char) * buf_size);
    char * current = buf;
    snprintf(buf, buf_size, "From: %s\n", post->author);
    current = &(current[strlen(post->author)+7]);
    snprintf(current, buf_size, "Date: %s\n", 
		asctime(localtime(post->date)));
	current = &(current[strlen(asctime(localtime(post->date)))+7]);
	snprintf(current, buf_size, "%s\n", post->contents);
    return buf;
}


/* 
 * Print a user profile.
 * For an example of the required output format, see the example output
 * linked from the handout.
 * Return:
 *   - 0 on success.
 *   - 1 if the user is NULL.
 */
char * print_user(const User *user) {
    if (user == NULL) {
        return NULL;
    }
    //strings used in print_user
    char * header = "Name: ";
    char * friends = "Friends:\n";
    char * dashes = "------------------------------------------\n";
    char * posts = "Posts:\n";
    
    //find the size of the array to return.
    int size =0;
    size += strlen(header);
    size += strlen(user->name) +2;
    size += strlen(dashes);
    size += strlen(friends);
    int i;
    for (i=0; MAX_FRIENDS && user->friends[i] != NULL; i++){
		size += strlen(user->friends[i]->name) + 1;
    }
    size += strlen(dashes);
    size += strlen(posts);
    Post * curr = user->first_post;
    while (curr != NULL) {
        size += strlen(print_post(curr));
        curr = curr->next;
        if (curr != NULL) {
            size += strlen("\n===\n\n");
        }
    }
    size += strlen(dashes);
    size += 1;

    
	//declare the size of the array
	char * print = malloc(sizeof(char) * (size+1));
	char * current = print;
	//copy "Name: " to print
	snprintf(print, size, "Name: %s\n\n", user->name);
	//strncpy(print, header, strlen(header));
	//int added = strlen(header);
	//int remainder = size - added;
	current = &(print[strlen(user->name)+8]);
	/*
	//copy user's name
	added = strlen(user->name);
	strncpy(current, user->name, remainder);
	remainder -= added;
	current = &(current[added]);*/
	
	//copy dashes
	snprintf(current, size, "%s", dashes);
	/*
	strncpy(current, dashes, remainder);
	added = strlen(dashes);
	remainder -= added;
	*/
	current = &(current[strlen(dashes)]);
	
	//copy friends 
	/*
	strncpy(current, friends, remainder);
	added = strlen(friends);
	remainder -= added;
	*/
	snprintf(current, size, "%s", friends);
	current = &(current[strlen(friends)]);
	
	//copy friends list
    for (i = 0; i < MAX_FRIENDS && user->friends[i] != NULL; i++) {
		snprintf(current, size, "%s\n", user->friends[i]->name);
		current = &(current[strlen(user->friends[i]->name)+1]);
		/*
		//add friend name
		added = strlen(user->friends[i]->name);
		strncpy(current, user->friends[i]->name, remainder);
		remainder -= added;
		current = &(current[added]);
		//add newline
		added = 1;
		strncpy(current, "\n", remainder);
		remainder -= added;
		current = &(current[added]);
		*/	
    }
    /*
    added = strlen(dashes);
    strncpy(current, dashes, remainder);
    remainder -= added;
    */
    snprintf(current, size, "%s", dashes);
    current = &(current[strlen(dashes)]);
    
    //add posts
    snprintf(current, size, "%s", posts);
    current = &(current[strlen(posts)]);
    /*
    added = strlen(posts);
    strncpy(current, posts, remainder);
    remainder -= added;
    current = &(current[added]);
    */
    
    curr = user->first_post;
    while (curr != NULL) {
		char * contents = print_post(curr);
		snprintf(current, size, "%s", contents);
		current = &(current[strlen(contents)]);
		free(contents);
        curr = curr->next;
        if (curr != NULL) {
			snprintf(current, size, "\n===\n\n");
			/*
			added = strlen(contents);
			strncpy(current, "\n===\n\n", remainder);
			remainder -= added;
			*/
			current = &(current[strlen("\n===\n\n")]);
        }
    }
    snprintf(current, size, "%s", dashes);
    current = &(current[strlen(dashes)]);
    print[size] = '\0';
    /*
    added = strlen(dashes);
    strncpy(current, dashes, remainder);
	remainder -= added;
	current = &(current[added]);
	*/
    /*
    // Print name
    printf("Name: %s\n\n", user->name);
    printf("------------------------------------------\n");

    // Print friend list.
    printf("Friends:\n");
    int i;
    for (i = 0; i < MAX_FRIENDS && user->friends[i] != NULL; i++) {
        printf("%s\n", user->friends[i]->name);
    }
    printf("------------------------------------------\n");

    // Print post list.
    printf("Posts:\n");
    const Post *curr = user->first_post;
    while (curr != NULL) {
        print_post(curr);
        curr = curr->next;
        if (curr != NULL) {
            printf("\n===\n\n");
        }
    }
    printf("------------------------------------------\n");
	*/
	
    return print;
}


/*
 * Make a new post from 'author' to the 'target' user,
 * containing the given contents, IF the users are friends.
 *
 * Insert the new post at the *front* of the user's list of posts.
 *
 * Use the 'time' function to store the current time.
 *
 * 'contents' is a pointer to heap-allocated memory - you do not need
 * to allocate more memory to store the contents of the post.
 *
 * Return:
 *   - 0 on success
 *   - 1 if users exist but are not friends
 *   - 2 if either User pointer is NULL
 */
int make_post(const User *author, User *target, char *contents) {
    if (target == NULL || author == NULL) {
        return 2;
    }

    int friends = 0;
    int i;
    for (i = 0; i < MAX_FRIENDS && target->friends[i] != NULL; i++) {
        if (strcmp(target->friends[i]->name, author->name) == 0) {
            friends = 1;
            break;
        }
    }

    if (friends == 0) {
        return 1;
    }

    // Create post
    Post *new_post = malloc(sizeof(Post));
    if (new_post == NULL) {
        perror("malloc");
        exit(1);
    }
    strncpy(new_post->author, author->name, MAX_NAME);
    new_post->contents = contents;
    new_post->date = malloc(sizeof(time_t));
    if (new_post->date == NULL) {
        perror("malloc");
        exit(1);
    }
    time(new_post->date);
    new_post->next = target->first_post;
    target->first_post = new_post;

    return 0;
}
