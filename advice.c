#include "cache.h"
#include "config.h"
#include "color.h"
#include "help.h"

int advice_fetch_show_forced_updates = 1;
int advice_push_update_rejected = 1;
int advice_push_non_ff_current = 1;
int advice_push_non_ff_matching = 1;
int advice_push_already_exists = 1;
int advice_push_fetch_first = 1;
int advice_push_needs_force = 1;
int advice_push_unqualified_ref_name = 1;
int advice_status_hints = 1;
int advice_status_u_option = 1;
int advice_status_ahead_behind_warning = 1;
int advice_commit_before_merge = 1;
int advice_reset_quiet_warning = 1;
int advice_resolve_conflict = 1;
int advice_sequencer_in_use = 1;
int advice_implicit_identity = 1;
int advice_detached_head = 1;
int advice_set_upstream_failure = 1;
int advice_object_name_warning = 1;
int advice_amworkdir = 1;
int advice_rm_hints = 1;
int advice_add_embedded_repo = 1;
int advice_ignored_hook = 1;
int advice_waiting_for_editor = 1;
int advice_graft_file_deprecated = 1;
int advice_checkout_ambiguous_remote_branch_name = 1;
int advice_submodule_alternate_error_strategy_die = 1;
int advice_add_ignored_file = 1;
int advice_add_empty_pathspec = 1;

static int advice_use_color = -1;
static char advice_colors[][COLOR_MAXLEN] = {
	GIT_COLOR_RESET,
	GIT_COLOR_YELLOW,	/* HINT */
};

enum color_advice {
	ADVICE_COLOR_RESET = 0,
	ADVICE_COLOR_HINT = 1,
};

static int parse_advise_color_slot(const char *slot)
{
	if (!strcasecmp(slot, "reset"))
		return ADVICE_COLOR_RESET;
	if (!strcasecmp(slot, "hint"))
		return ADVICE_COLOR_HINT;
	return -1;
}

static const char *advise_get_color(enum color_advice ix)
{
	if (want_color_stderr(advice_use_color))
		return advice_colors[ix];
	return "";
}

static struct {
	const char *name;
	int *preference;
} advice_config[] = {
	{ "fetchShowForcedUpdates", &advice_fetch_show_forced_updates },
	{ "pushUpdateRejected", &advice_push_update_rejected },
	{ "pushNonFFCurrent", &advice_push_non_ff_current },
	{ "pushNonFFMatching", &advice_push_non_ff_matching },
	{ "pushAlreadyExists", &advice_push_already_exists },
	{ "pushFetchFirst", &advice_push_fetch_first },
	{ "pushNeedsForce", &advice_push_needs_force },
	{ "pushUnqualifiedRefName", &advice_push_unqualified_ref_name },
	{ "statusHints", &advice_status_hints },
	{ "statusUoption", &advice_status_u_option },
	{ "statusAheadBehindWarning", &advice_status_ahead_behind_warning },
	{ "commitBeforeMerge", &advice_commit_before_merge },
	{ "resetQuiet", &advice_reset_quiet_warning },
	{ "resolveConflict", &advice_resolve_conflict },
	{ "sequencerInUse", &advice_sequencer_in_use },
	{ "implicitIdentity", &advice_implicit_identity },
	{ "detachedHead", &advice_detached_head },
	{ "setupStreamFailure", &advice_set_upstream_failure },
	{ "objectNameWarning", &advice_object_name_warning },
	{ "amWorkDir", &advice_amworkdir },
	{ "rmHints", &advice_rm_hints },
	{ "addEmbeddedRepo", &advice_add_embedded_repo },
	{ "ignoredHook", &advice_ignored_hook },
	{ "waitingForEditor", &advice_waiting_for_editor },
	{ "graftFileDeprecated", &advice_graft_file_deprecated },
	{ "checkoutAmbiguousRemoteBranchName", &advice_checkout_ambiguous_remote_branch_name },
	{ "submoduleAlternateErrorStrategyDie", &advice_submodule_alternate_error_strategy_die },
	{ "addIgnoredFile", &advice_add_ignored_file },
	{ "addEmptyPathspec", &advice_add_empty_pathspec },

	/* make this an alias for backward compatibility */
	{ "pushNonFastForward", &advice_push_update_rejected }
};

static const char *advice_config_keys[] = {
	[ADD_EMBEDDED_REPO]			 = "addEmbeddedRepo",
	[AMWORKDIR]				 = "amWorkDir",
	[CHECKOUT_AMBIGUOUS_REMOTE_BRANCH_NAME]	 = "checkoutAmbiguousRemoteBranchName",
	[COMMIT_BEFORE_MERGE]			 = "commitBeforeMerge",
	[DETACHED_HEAD]				 = "detachedHead",
	[FETCH_SHOW_FORCED_UPDATES]		 = "fetchShowForcedUpdates",
	[GRAFT_FILE_DEPRECATED]			 = "graftFileDeprecated",
	[IGNORED_HOOK]				 = "ignoredHook",
	[IMPLICIT_IDENTITY]			 = "implicitIdentity",
	[NESTED_TAG]				 = "nestedTag",
	[OBJECT_NAME_WARNING]			 = "objectNameWarning",
	[PUSH_ALREADY_EXISTS]			 = "pushAlreadyExists",
	[PUSH_FETCH_FIRST]			 = "pushFetchFirst",
	[PUSH_NEEDS_FORCE]			 = "pushNeedsForce",

	/* make this an alias for backward compatibility */
	[PUSH_UPDATE_REJECTED_ALIAS]		 = "pushNonFastForward",

	[PUSH_NON_FF_CURRENT]			 = "pushNonFFCurrent",
	[PUSH_NON_FF_MATCHING]			 = "pushNonFFMatching",
	[PUSH_UNQUALIFIED_REF_NAME]		 = "pushUnqualifiedRefName",
	[PUSH_UPDATE_REJECTED]			 = "pushUpdateRejected",
	[RESET_QUIET_WARNING]			 = "resetQuiet",
	[RESOLVE_CONFLICT]			 = "resolveConflict",
	[RM_HINTS]				 = "rmHints",
	[SEQUENCER_IN_USE]			 = "sequencerInUse",
	[SET_UPSTREAM_FAILURE]			 = "setupStreamFailure",
	[STATUS_AHEAD_BEHIND_WARNING]		 = "statusAheadBehindWarning",
	[STATUS_HINTS]				 = "statusHints",
	[STATUS_U_OPTION]			 = "statusUoption",
	[SUBMODULE_ALTERNATE_ERROR_STRATEGY_DIE] = "submoduleAlternateErrorStrategyDie",
	[WAITING_FOR_EDITOR] 			 = "waitingForEditor",
};

static const char turn_off_instructions[] =
N_("\n"
   "Disable this message with \"git config %s false\"");

static void vadvise(const char *advice, int display_instructions,
		    char *key, va_list params)
{
	struct strbuf buf = STRBUF_INIT;
	const char *cp, *np;

	strbuf_vaddf(&buf, advice, params);

	if (display_instructions)
		strbuf_addf(&buf, turn_off_instructions, key);

	for (cp = buf.buf; *cp; cp = np) {
		np = strchrnul(cp, '\n');
		fprintf(stderr,	_("%shint: %.*s%s\n"),
			advise_get_color(ADVICE_COLOR_HINT),
			(int)(np - cp), cp,
			advise_get_color(ADVICE_COLOR_RESET));
		if (*np)
			np++;
	}
	strbuf_release(&buf);
}

void advise(const char *advice, ...)
{
	va_list params;
	va_start(params, advice);
	vadvise(advice, 0, "", params);
	va_end(params);
}

static int get_config_value(enum advice_type type)
{
	int value = 1;
	char *key = xstrfmt("%s.%s", "advice", advice_config_keys[type]);

	git_config_get_bool(key, &value);
	free(key);
	return value;
}

int advice_enabled(enum advice_type type)
{
	switch (type) {
	case PUSH_UPDATE_REJECTED:
		return get_config_value(PUSH_UPDATE_REJECTED) &&
		       get_config_value(PUSH_UPDATE_REJECTED_ALIAS);
	default:
		return get_config_value(type);
	}
}

void advise_if_enabled(enum advice_type type, const char *advice, ...)
{
	char *key = xstrfmt("%s.%s", "advice", advice_config_keys[type]);
	va_list params;

	if (!advice_enabled(type))
		return;

	va_start(params, advice);
	vadvise(advice, 1, key, params);
	va_end(params);
	free(key);
}

int git_default_advice_config(const char *var, const char *value)
{
	const char *k, *slot_name;
	int i;

	if (!strcmp(var, "color.advice")) {
		advice_use_color = git_config_colorbool(var, value);
		return 0;
	}

	if (skip_prefix(var, "color.advice.", &slot_name)) {
		int slot = parse_advise_color_slot(slot_name);
		if (slot < 0)
			return 0;
		if (!value)
			return config_error_nonbool(var);
		return color_parse(value, advice_colors[slot]);
	}

	if (!skip_prefix(var, "advice.", &k))
		return 0;

	for (i = 0; i < ARRAY_SIZE(advice_config); i++) {
		if (strcasecmp(k, advice_config[i].name))
			continue;
		*advice_config[i].preference = git_config_bool(var, value);
		return 0;
	}

	return 0;
}

void list_config_advices(struct string_list *list, const char *prefix)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(advice_config_keys); i++)
		list_config_item(list, prefix, advice_config_keys[i]);
}

int error_resolve_conflict(const char *me)
{
	if (!strcmp(me, "cherry-pick"))
		error(_("Cherry-picking is not possible because you have unmerged files."));
	else if (!strcmp(me, "commit"))
		error(_("Committing is not possible because you have unmerged files."));
	else if (!strcmp(me, "merge"))
		error(_("Merging is not possible because you have unmerged files."));
	else if (!strcmp(me, "pull"))
		error(_("Pulling is not possible because you have unmerged files."));
	else if (!strcmp(me, "revert"))
		error(_("Reverting is not possible because you have unmerged files."));
	else
		error(_("It is not possible to %s because you have unmerged files."),
			me);

	if (advice_resolve_conflict)
		/*
		 * Message used both when 'git commit' fails and when
		 * other commands doing a merge do.
		 */
		advise(_("Fix them up in the work tree, and then use 'git add/rm <file>'\n"
			 "as appropriate to mark resolution and make a commit."));
	return -1;
}

void NORETURN die_resolve_conflict(const char *me)
{
	error_resolve_conflict(me);
	die(_("Exiting because of an unresolved conflict."));
}

void NORETURN die_conclude_merge(void)
{
	error(_("You have not concluded your merge (MERGE_HEAD exists)."));
	if (advice_resolve_conflict)
		advise(_("Please, commit your changes before merging."));
	die(_("Exiting because of unfinished merge."));
}

void detach_advice(const char *new_name)
{
	const char *fmt =
	_("Note: switching to '%s'.\n"
	"\n"
	"You are in 'detached HEAD' state. You can look around, make experimental\n"
	"changes and commit them, and you can discard any commits you make in this\n"
	"state without impacting any branches by switching back to a branch.\n"
	"\n"
	"If you want to create a new branch to retain commits you create, you may\n"
	"do so (now or later) by using -c with the switch command. Example:\n"
	"\n"
	"  git switch -c <new-branch-name>\n"
	"\n"
	"Or undo this operation with:\n"
	"\n"
	"  git switch -\n"
	"\n"
	"Turn off this advice by setting config variable advice.detachedHead to false\n\n");

	fprintf(stderr, fmt, new_name);
}
